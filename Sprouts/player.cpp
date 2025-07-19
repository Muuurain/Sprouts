#include "player.h"
#include "spritegroup.h"
#include "resourceloader.h"
#include "tree.h"
#include <QKeyEvent>
#include <QDebug>
#include <QtMath>
#include <QUrl>

Player::Player(const QPointF& pos, SpriteGroup* group, 
               SpriteGroup* collisionSprites, SpriteGroup* treeSprites,
               SpriteGroup* interactionSprites, SoilLayer* soilLayer,
               std::function<void()> toggleShop, QObject *parent)
    : Sprite(parent), status("down_idle"), frameIndex(0), direction(0, 0), 
      speed(200), toolIndex(0), seedIndex(0), money(200), energy(100), maxEnergy(100), sleep(false),
      collisionSprites(collisionSprites), treeSprites(treeSprites),
      interactionSprites(interactionSprites), soilLayer(soilLayer),
      toggleShop(toggleShop)
{
    // Import assets
    importAssets();
    
    // Setup initial state
    if (animations.contains(status) && !animations[status].isEmpty()) {
        image = animations[status][0];
    } else {
        // Create a placeholder image if no animation is found
        image = QPixmap(64, 64);
        image.fill(Qt::red);
    }
    
    rect = QRect(pos.toPoint() - QPoint(image.width()/2, image.height()/2), image.size());
    this->pos = QPointF(rect.center());
    z = MAIN;
    
    // Setup hitbox (smaller than the sprite for better collision detection)
    // Make hitbox smaller and position it at the bottom center of the sprite
    int hitboxWidth = rect.width() * 0.3;  // Much smaller width
    int hitboxHeight = rect.height() * 0.15; // Much smaller height
    hitbox = QRect(rect.x() + (rect.width() - hitboxWidth) / 2,
                   rect.y() + rect.height() - hitboxHeight,
                   hitboxWidth, hitboxHeight);
    
    // Setup tools and seeds
    tools = {"hoe", "axe", "water"};
    selectedTool = tools[toolIndex];
    
    seeds = {"corn", "tomato"};
    selectedSeed = seeds[seedIndex];
    
    // Setup inventory
    inventory["wood"] = 20;
    inventory["apple"] = 20;
    inventory["corn"] = 20;
    inventory["tomato"] = 20;
    
    seedInventory["corn"] = 5;
    seedInventory["tomato"] = 5;
    
    // Setup timers
    setupTimers();
    
    // Setup sound
    wateringSound = new QSoundEffect(this);
    QString waterPath = ResourceLoader::getResourcePath("audio/water.wav");
    wateringSound->setSource(QUrl::fromLocalFile(waterPath));
    wateringSound->setVolume(0.2);
    
    // Add to group
    if (group) {
        addToGroup(group);
    }
    
    // Check for initial collision and adjust position if needed
    checkInitialPosition();
}

void Player::setupTimers()
{
    timers["tool use"] = new GameTimer(350, [this]() { useTool(); }, this);
    timers["tool switch"] = new GameTimer(200, nullptr, this);
    timers["seed use"] = new GameTimer(350, [this]() { useSeed(); }, this);
    timers["seed switch"] = new GameTimer(200, nullptr, this);
    timers["interaction"] = new GameTimer(300, nullptr, this);
}

void Player::checkInitialPosition()
{
    // Check if player's initial position overlaps with any collision sprite
    if (!collisionSprites) {
        return;
    }
    
    bool hasCollision = false;
    for (Sprite* sprite : collisionSprites->sprites()) {
        if (sprite->hitbox.intersects(hitbox)) {
            hasCollision = true;
            break;
        }
    }
    
    if (!hasCollision) {
        return; // No collision, position is fine
    }
    
    qDebug() << "Player: Initial position collision detected, searching for safe position";
    QPoint originalPos = hitbox.center();
    
    // Try different offsets to find a safe position
    QVector<QPoint> offsets = {
        QPoint(100, 0),   // Right
        QPoint(-100, 0),  // Left
        QPoint(0, -100),  // Up
        QPoint(0, 100),   // Down
        QPoint(150, 0),   // Further right
        QPoint(-150, 0),  // Further left
        QPoint(0, -150),  // Further up
        QPoint(0, 150),   // Further down
        QPoint(100, -100), // Right-up
        QPoint(-100, -100), // Left-up
        QPoint(100, 100),  // Right-down
        QPoint(-100, 100)  // Left-down
    };
    
    for (const QPoint& offset : offsets) {
        QPoint testPos = originalPos + offset;
        hitbox.moveCenter(testPos);
        
        // Check if this position is collision-free
        bool isCollisionFree = true;
        for (Sprite* checkSprite : collisionSprites->sprites()) {
            if (checkSprite->hitbox.intersects(hitbox)) {
                isCollisionFree = false;
                break;
            }
        }
        
        if (isCollisionFree) {
            // Found a safe position
            rect.moveCenter(hitbox.center());
            pos = QPointF(rect.center());
            qDebug() << "Player: Position adjusted to" << pos;
            return;
        }
    }
    
    // If no safe position found, revert to original and warn
    hitbox.moveCenter(originalPos);
    rect.moveCenter(hitbox.center());
    pos = QPointF(rect.center());
    qDebug() << "Player: Warning - Could not find collision-free position, staying at" << pos;
}

void Player::importAssets()
{
    QStringList animationTypes = {
        "up", "down", "left", "right",
        "right_idle", "left_idle", "up_idle", "down_idle",
        "right_hoe", "left_hoe", "up_hoe", "down_hoe",
        "right_axe", "left_axe", "up_axe", "down_axe",
        "right_water", "left_water", "up_water", "down_water"
    };
    
    for (const QString& animationType : animationTypes) {
        QString fullPath = QString("graphics/character/%1").arg(animationType);
        animations[animationType] = ResourceLoader::importFolder(fullPath);
    }
}

void Player::animate(float dt)
{
    if (!animations.contains(status) || animations[status].isEmpty()) {
        return;
    }
    
    frameIndex += 4.0f * dt;
    if (frameIndex >= animations[status].size()) {
        frameIndex = 0;
    }
    
    image = animations[status][static_cast<int>(frameIndex)];
}

void Player::handleInput(const QList<int>& pressedKeys)
{
    if (timers["tool use"]->isActive() || sleep) {
        return;
    }
    
    // Handle input
    
    // Reset direction
    direction = QPointF(0, 0);
    
    // Movement input
    if (pressedKeys.contains(Qt::Key_Up)) {
        direction.setY(-1);
        status = "up";
    } else if (pressedKeys.contains(Qt::Key_Down)) {
        direction.setY(1);
        status = "down";
    }
    
    if (pressedKeys.contains(Qt::Key_Right)) {
        direction.setX(1);
        status = "right";
    } else if (pressedKeys.contains(Qt::Key_Left)) {
        direction.setX(-1);
        status = "left";
    }
    
    // Tool use
    if (pressedKeys.contains(Qt::Key_Space)) {
        timers["tool use"]->activate();
        direction = QPointF(0, 0);
        frameIndex = 0;
    }
    
    // Change tool
    if (pressedKeys.contains(Qt::Key_Q) && !timers["tool switch"]->isActive()) {
        timers["tool switch"]->activate();
        toolIndex = (toolIndex + 1) % tools.size();
        selectedTool = tools[toolIndex];
    }
    
    // Seed use
    if (pressedKeys.contains(Qt::Key_Control)) {
        timers["seed use"]->activate();
        direction = QPointF(0, 0);
        frameIndex = 0;
    }
    
    // Change seed
    if (pressedKeys.contains(Qt::Key_E) && !timers["seed switch"]->isActive()) {
        timers["seed switch"]->activate();
        seedIndex = (seedIndex + 1) % seeds.size();
        selectedSeed = seeds[seedIndex];
    }
    
    // Interaction
    if (pressedKeys.contains(Qt::Key_Return) && !timers["interaction"]->isActive()) {
        // Check for interaction sprites collision
        if (interactionSprites) {
            for (Sprite* sprite : interactionSprites->sprites()) {
                Interaction* interaction = qobject_cast<Interaction*>(sprite);
                if (interaction && interaction->rect.intersects(rect)) {
                    if (interaction->name == "Trader") {
                        toggleShop();
                        timers["interaction"]->activate();
                    } else {
                        status = "left_idle";
                        sleep = true;
                        timers["interaction"]->activate();
                    }
                    break;
                }
            }
        }
    }
}

void Player::useTool()
{
    // Check if player has enough energy to use tools
    if (energy <= 0) {
        return;
    }
    
    getTargetPos();
    
    if (selectedTool == "hoe") {
        if (soilLayer) {
            soilLayer->getHit(targetPos);
            decreaseEnergy(1); // Using hoe consumes 1 energy
        }
    } else if (selectedTool == "axe") {
        // Check tree collision
        if (treeSprites) {
            for (Sprite* sprite : treeSprites->sprites()) {
                Tree* tree = qobject_cast<Tree*>(sprite);
                if (tree && tree->rect.contains(targetPos.toPoint())) {
                    tree->damage();
                    decreaseEnergy(2); // Using axe consumes 2 energy
                    break;
                }
            }
        }
    } else if (selectedTool == "water") {
        if (soilLayer) {
            soilLayer->water(targetPos);
            decreaseEnergy(1); // Using water consumes 1 energy
        }
        if (wateringSound) {
            wateringSound->play();
        }
    }
}

void Player::useSeed()
{
    if (seedInventory[selectedSeed] > 0 && energy > 0) {
        getTargetPos();
        if (soilLayer) {
            if (soilLayer->plantSeed(targetPos, selectedSeed)) {
                seedInventory[selectedSeed]--;
                decreaseEnergy(2); // Planting consumes 2 energy
            }
        }
    }
}

void Player::getTargetPos()
{
    QString direction = getDirectionFromStatus();
    if (PLAYER_TOOL_OFFSET.contains(direction)) {
        targetPos = QPointF(rect.center()) + PLAYER_TOOL_OFFSET[direction];
    } else {
        targetPos = QPointF(rect.center());
    }
}

QString Player::getDirectionFromStatus() const
{
    if (status.contains("_")) {
        return status.split("_")[0];
    }
    return status;
}

void Player::getStatus()
{
    // Idle status
    if (qFuzzyIsNull(direction.x()) && qFuzzyIsNull(direction.y())) {
        QString baseDirection = getDirectionFromStatus();
        status = baseDirection + "_idle";
    }
    
    // Tool use status
    if (timers["tool use"]->isActive()) {
        QString baseDirection = getDirectionFromStatus();
        status = baseDirection + "_" + selectedTool;
    }
}

void Player::updateTimers()
{
    for (GameTimer* timer : timers.values()) {
        timer->update();
    }
}

void Player::collision(const QString& direction)
{
    // Check collision sprites only - let TMX collision data define all boundaries
    if (!collisionSprites) {
        return;
    }
    
    // Only check collision if actually moving in the specified direction
    if (direction == "horizontal" && qFuzzyIsNull(this->direction.x())) {
        return;
    }
    if (direction == "vertical" && qFuzzyIsNull(this->direction.y())) {
        return;
    }
    
    for (Sprite* sprite : collisionSprites->sprites()) {
        if (sprite->hitbox.intersects(hitbox)) {
            if (direction == "horizontal") {
                if (this->direction.x() > 0) { // moving right
                    hitbox.moveRight(sprite->hitbox.left() - 1);
                } else if (this->direction.x() < 0) { // moving left
                    hitbox.moveLeft(sprite->hitbox.right() + 1);
                }
                rect.moveCenter(hitbox.center());
                pos = QPointF(rect.center());
                break; // Exit after first collision to prevent multiple adjustments
            } else if (direction == "vertical") {
                if (this->direction.y() > 0) { // moving down
                    hitbox.moveBottom(sprite->hitbox.top() - 1);
                } else if (this->direction.y() < 0) { // moving up
                    hitbox.moveTop(sprite->hitbox.bottom() + 1);
                }
                rect.moveCenter(hitbox.center());
                pos = QPointF(rect.center());
                break; // Exit after first collision to prevent multiple adjustments
            }
        }
    }
}

void Player::move(float dt)
{
    // Normalize direction
    if (!qFuzzyIsNull(direction.x()) || !qFuzzyIsNull(direction.y())) {
        float magnitude = qSqrt(direction.x() * direction.x() + direction.y() * direction.y());
        if (magnitude > 0) {
            direction /= magnitude;
        }
    }
    
    // Horizontal movement
    pos.setX(pos.x() + direction.x() * speed * dt);
    hitbox.moveCenter(QPoint(qRound(pos.x()), hitbox.center().y()));
    rect.moveCenter(hitbox.center());
    collision("horizontal");
    
    // Vertical movement
    pos.setY(pos.y() + direction.y() * speed * dt);
    hitbox.moveCenter(QPoint(hitbox.center().x(), qRound(pos.y())));
    rect.moveCenter(hitbox.center());
    collision("vertical");
}

void Player::update(float dt)
{
    updateTimers();
    getStatus();
    move(dt);
    animate(dt);
}

void Player::restoreEnergy()
{
    energy = maxEnergy;
}

void Player::decreaseEnergy(int amount)
{
    energy = qMax(0, energy - amount);
}