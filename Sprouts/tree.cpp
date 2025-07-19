#include "tree.h"
#include "resourceloader.h"
#include <QRandomGenerator>
#include <QDebug>

Tree::Tree(const QPoint& pos, const QPixmap& surf, QVector<SpriteGroup*> groups, 
           const QString& name, std::function<void(const QString&)> playerAdd, QObject *parent)
    : Generic(pos, surf, groups), treeName(name), playerAdd(playerAdd), health(5), alive(true)
{
    setParent(parent);
    
    // Load stump surface
    QString stumpPath = QString("graphics/stumps/%1.png").arg(name == "Small" ? "small" : "large");
    stumpSurf = ResourceLoader::loadImage(stumpPath);
    
    // Load apple surface
    appleSurf = ResourceLoader::loadImage("graphics/fruit/apple.png");
    
    // Get apple positions for this tree type
    if (APPLE_POS.contains(name)) {
        applePos = APPLE_POS[name];
    }
    
    // Create apple sprite group
    appleSprites = new SpriteGroup(this);
    
    // Create initial fruit
    createFruit();
    
    // Setup sound
    axeSound = new QSoundEffect(this);
    QString axePath = ResourceLoader::getResourcePath("audio/axe.wav");
    axeSound->setSource(QUrl::fromLocalFile(axePath));
    axeSound->setVolume(0.5);
}

void Tree::damage()
{
    // Damage the tree
    health--;
    
    // Play sound
    if (axeSound) {
        axeSound->play();
    }
    
    // Remove an apple if available
    QVector<Sprite*> apples = appleSprites->sprites();
    if (!apples.isEmpty()) {
        // Choose random apple
        int randomIndex = QRandomGenerator::global()->bounded(apples.size());
        Sprite* randomApple = apples[randomIndex];
        
        // Create particle effect
        if (randomApple) {
            // Create particle effect for apple drop
            QVector<SpriteGroup*> particleGroups;
            particleGroups.append(groups[0]); // Use the first group (usually allSprites)
            Particle* particle = new Particle(randomApple->rect.topLeft(), randomApple->image, 
                                             particleGroups, FRUIT);
            particle->setParent(this->parent());
            
            playerAdd("apple");
            randomApple->kill();
        }
    }
}

void Tree::checkDeath()
{
    if (health <= 0) {
        // Create particle effect for tree death
        // Particle(rect.topLeft(), image, groups()[0], FRUIT, 300);
        
        // Change to stump
        image = stumpSurf;
        QPoint bottomCenter = rect.bottomLeft() + QPoint(rect.width() / 2, 0);
        rect = QRect(bottomCenter - QPoint(stumpSurf.width() / 2, stumpSurf.height()),
                     stumpSurf.size());
        
        // Adjust hitbox
        int hitboxWidth = rect.width() - 10;
        int hitboxHeight = rect.height() * 0.4;
        hitbox = QRect(rect.x() + 5,
                       rect.y() + rect.height() - hitboxHeight,
                       hitboxWidth, hitboxHeight);
        
        alive = false;
        playerAdd("wood");
    }
}

void Tree::update(float dt)
{
    if (alive) {
        checkDeath();
    }
    
    // Update apple sprites
    if (appleSprites) {
        appleSprites->update(dt);
    }
}

void Tree::createFruit()
{
    if (!appleSprites) return;
    
    for (const QPoint& pos : applePos) {
        // Random chance to create apple
        if (QRandomGenerator::global()->bounded(11) < 2) { // 20% chance
            QPoint appleWorldPos = rect.topLeft() + pos;
            
            // Create apple sprite
            QVector<SpriteGroup*> appleGroups;
            appleGroups.append(appleSprites);
            // Also add to main sprite group if available
            if (!groups.isEmpty()) {
                appleGroups.append(groups[0]);
            }
            
            Generic* apple = new Generic(appleWorldPos, appleSurf, appleGroups, FRUIT);
            apple->setParent(this);
        }
    }
}