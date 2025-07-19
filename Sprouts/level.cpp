#include "level.h"
#include "player.h"
#include "spritegroup.h"
#include "sprite.h"
#include "tree.h"
#include "soillayer.h"
#include "plant.h"
#include "overlay.h"
#include "transition.h"
#include "sky.h"
#include "menu.h"
#include "introanimation.h"
#include "endinganimation.h"
#include "resourceloader.h"
#include <QRandomGenerator>
#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>
#include <QStringList>
#include <QCoreApplication>
#include <QDir>
#include <algorithm>

Level::Level(QObject *parent)
    : QObject{parent}, shopActive(false), raining(false), energyTimer(0.0f), energyDecreaseInterval(10.0f),
      currentDay(1), currentTime(6.0f), timeSpeed(0.5f), isRaining(false)
{


    // Initialize sprite groups
    allSprites = new CameraGroup(this);
    collisionSprites = new SpriteGroup(this);
    treeSprites = new SpriteGroup(this);
    interactionSprites = new SpriteGroup(this);


    // Initialize soil layer
    soilLayer = new SoilLayer(allSprites, collisionSprites, this);


    // Setup the level
    setup();


    // Initialize other components
    overlay = new Overlay(player, this);
    transition = new Transition([this]() { reset(); }, player, this);


    // Initialize weather
    rain = new Rain(allSprites, this);
    isRaining = QRandomGenerator::global()->bounded(11) > 7; // 30% chance of rain on first day
    raining = isRaining;
    soilLayer->raining = raining;
    if (raining) {
        soilLayer->waterAll();
    }
    sky = new Sky(this);


    // Initialize shop
    menu = new Menu(player, [this]() { toggleShop(); }, [this]() {
        // Trigger success ending
        float totalHours = (currentDay - 1) * 24.0f + currentTime;
        endingAnimation->start(EndingType::SUCCESS, currentDay, totalHours);
    }, this);

    // Initialize intro animation
    introAnimation = new IntroAnimation([this]() {
        // Animation completed callback - do nothing for now
    }, this);
    introAnimation->start();
    
    // Initialize ending animation
    endingAnimation = new EndingAnimation(nullptr, this);

    // Setup audio
    setupAudio();

}

Level::~Level()
{
    // Cleanup is handled by Qt's parent-child system
}

void Level::setup()
{
    // Load TMX map data (simplified)
    loadTMXMap();

    // Create ground
    QPixmap groundSurf = ResourceLoader::loadImage("graphics/world/ground.png");
    if (!groundSurf.isNull()) {

        QVector<SpriteGroup*> groundGroups;
        groundGroups.append(allSprites);
        Generic* ground = new Generic(QPoint(0, 0), groundSurf, groundGroups, GROUND);
        ground->setParent(this);

    } else {

    }
}

void Level::parseTMXCollisionLayer()
{
    // Parse TMX collision layer data from the actual TMX file
    QString tmxFilePath = ResourceLoader::getResourcePath("data/map.tmx");
    QFile file(tmxFilePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open TMX file:" << tmxFilePath;
        return;
    }

    QXmlStreamReader xml(&file);
    int collisionTileCount = 0;
    int fenceCount = 0;
    int treeCount = 0;
    int decorationCount = 0;

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();

        if (token == QXmlStreamReader::StartElement) {
            // Parse Collision layer
            if (xml.name() == QLatin1String("layer")) {
                QXmlStreamAttributes attributes = xml.attributes();
                QString layerName = attributes.value("name").toString();

                if (layerName == "Collision" || layerName == "Fence") {
                    // Find the data element for this layer
                    while (!xml.atEnd() && !(xml.isStartElement() && xml.name() == QLatin1String("data"))) {
                        xml.readNext();
                    }

                    if (xml.isStartElement() && xml.name() == QLatin1String("data")) {
                        QString csvData = xml.readElementText().trimmed();
                        QStringList lines = csvData.split('\n', Qt::SkipEmptyParts);

                        for (int y = 0; y < lines.size(); ++y) {
                            QStringList values = lines[y].split(',', Qt::SkipEmptyParts);
                            for (int x = 0; x < values.size(); ++x) {
                                int tileId = values[x].trimmed().toInt();
                                // Create collision for specific conditions
                                bool shouldCreateCollision = false;
                                if (layerName == "Collision" && tileId == 170) {
                                    shouldCreateCollision = true;
                                } else if (layerName == "Fence" && tileId != 0) {
                                    shouldCreateCollision = true;
                                }

                                if (shouldCreateCollision) {
                                    QPixmap collisionSurf(TILE_SIZE, TILE_SIZE);
                                    collisionSurf.fill(Qt::transparent);

                                    QVector<SpriteGroup*> groups;
                                    if (layerName == "Fence") {
                                        groups.append(allSprites);
                                        groups.append(collisionSprites);
                                        fenceCount++;
                                    } else {
                                        groups.append(collisionSprites);
                                        collisionTileCount++;
                                    }

                                    Generic* collisionTile = new Generic(QPoint(x * TILE_SIZE, y * TILE_SIZE),
                                                                        collisionSurf, groups, MAIN);
                                    collisionTile->hitbox = QRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
                                    collisionTile->setParent(this);
                                }
                            }
                        }
                    }
                }
            }
            // Parse Trees and Decoration objectgroups
            else if (xml.name() == QLatin1String("objectgroup")) {
                QXmlStreamAttributes attributes = xml.attributes();
                QString groupName = attributes.value("name").toString();

                if (groupName == "Trees" || groupName == "Decoration") {
                     while (!xml.atEnd() && xml.name() != QLatin1String("objectgroup")) {
                         xml.readNext();
                         if (xml.isStartElement() && xml.name() == QLatin1String("object")) {
                            QXmlStreamAttributes objAttrs = xml.attributes();
                            int x = objAttrs.value("x").toInt();
                            int y = objAttrs.value("y").toInt();
                            int width = objAttrs.value("width").toInt();
                            int height = objAttrs.value("height").toInt();

                            QPixmap objSurf(width, height);
                            objSurf.fill(Qt::transparent);

                            QVector<SpriteGroup*> groups;
                            groups.append(allSprites);
                            groups.append(collisionSprites);

                            if (groupName == "Trees") {
                                groups.append(treeSprites);
                                treeCount++;
                            } else {
                                decorationCount++;
                            }

                            Generic* objSprite = new Generic(QPoint(x, y - height), objSurf, groups, MAIN);
                            objSprite->hitbox = QRect(x, y - height, width, height);
                            objSprite->setParent(this);
                        }
                    }
                }
            }
        }
    }

    file.close();

    if (xml.hasError()) {

        return;
    }


}

void Level::loadTilesets()
{
    // Parse TSX files to get correct tileset information
    QMap<QString, int> tilesetFiles = {
        {"data/Tilesets/Grass.tsx", 1},
        {"data/Tilesets/Hills.tsx", 81},
        {"data/Tilesets/Fences.tsx", 117},
        {"data/Tilesets/Plant Decoration.tsx", 133},
        {"data/Tilesets/Objects.tsx", 143},
        {"data/Tilesets/Paths.tsx", 153},
        {"data/Tilesets/interaction.tsx", 169},
        {"data/Tilesets/Water.tsx", 171},
        {"data/Tilesets/House.tsx", 172},
        {"data/Tilesets/House Decoration.tsx", 207}
    };
    
    for (auto it = tilesetFiles.begin(); it != tilesetFiles.end(); ++it) {
        QString tsxPath = it.key();
        int firstGid = it.value();
        
        // Use ResourceLoader to get the correct path
        QString fullTsxPath = ResourceLoader::getResourcePath(tsxPath);
        QFile tsxFile(fullTsxPath);
        
        if (tsxFile.exists() && tsxFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QXmlStreamReader xml(&tsxFile);
            bool hasIndividualTiles = false;
            int currentTileId = 0;
            
            while (!xml.atEnd() && !xml.hasError()) {
                xml.readNext();
                
                if (xml.isStartElement()) {
                    if (xml.name() == QLatin1String("tileset")) {
                        // Check if this tileset has individual tiles (columns=0)
                        QXmlStreamAttributes attrs = xml.attributes();
                        int columns = attrs.value("columns").toInt();
                        hasIndividualTiles = (columns == 0);
                    }
                    else if (xml.name() == QLatin1String("tile") && hasIndividualTiles) {
                        QXmlStreamAttributes attrs = xml.attributes();
                        currentTileId = attrs.value("id").toInt();
                    }
                    else if (xml.name() == QLatin1String("image")) {
                        QXmlStreamAttributes attrs = xml.attributes();
                        QString imagePath = attrs.value("source").toString();
                        
                        // Convert relative path to absolute
                        if (imagePath.startsWith("../../")) {
                            imagePath = imagePath.mid(6); // Remove "../../"
                        }
                        
                        QPixmap tileImage = ResourceLoader::loadImage(imagePath);
                        if (!tileImage.isNull()) {
                            if (hasIndividualTiles) {
                                // Store individual tile image
                                int globalTileId = firstGid + currentTileId;
                                individualTileImages[globalTileId] = tileImage;
                            } else {
                                // Store tileset image
                                tilesetImages[firstGid] = tileImage;
                            }
                        }
                    }
                }
            }
            
            tsxFile.close();
        } else {
            qDebug() << "Failed to open TSX file:" << fullTsxPath;
        }
    }
}

void Level::parseTMXVisualLayers()
{
    QString tmxFilePath = ResourceLoader::getResourcePath("data/map.tmx");
    QFile file(tmxFilePath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open TMX file for visual layers:" << tmxFilePath;
        return;
    }
    
    QXmlStreamReader xml(&file);
    
    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        
        if (token == QXmlStreamReader::StartElement && xml.name() == QLatin1String("layer")) {
            QXmlStreamAttributes attributes = xml.attributes();
            QString layerName = attributes.value("name").toString();
            
            // Skip only the Collision layer as it's handled separately
             if (layerName == "Collision") {
                 // Skip to the end of this layer
                 while (!xml.atEnd() && !(xml.isEndElement() && xml.name() == QLatin1String("layer"))) {
                     xml.readNext();
                 }
                 continue;
             }
            
            // Parse visual layers (Water, HouseFloor, HouseFurniture, etc.)
            while (!xml.atEnd() && !(xml.isStartElement() && xml.name() == QLatin1String("data"))) {
                xml.readNext();
            }
            
            if (xml.isStartElement() && xml.name() == QLatin1String("data")) {
                QString csvData = xml.readElementText().trimmed();
                QStringList lines = csvData.split('\n', Qt::SkipEmptyParts);
                
                QVector<QVector<int>> layerData;
                for (const QString& line : lines) {
                    QStringList values = line.split(',', Qt::SkipEmptyParts);
                    QVector<int> rowData;
                    for (const QString& value : values) {
                        rowData.append(value.trimmed().toInt());
                    }
                    layerData.append(rowData);
                }
                
                mapLayers.append(layerData);
                layerNames.append(layerName);
            }
        }
    }
    
    file.close();
    
    if (xml.hasError()) {

    }
}

void Level::renderTMXLayers(QPainter& painter, const QPointF& offset)
{

    // Define the correct rendering order for layers
    QStringList renderOrder = {
        "Water", "Ground", "Hills", "Forest Grass", "Outside Decoration",
        "HouseFloor", "HouseWalls", "Fence", "HouseFurnitureBottom", "HouseFurnitureTop"
    };
    
    // Render layers in the correct order
    for (const QString& targetLayerName : renderOrder) {
        // Find the layer index for this layer name
        int layerIndex = -1;
        for (int i = 0; i < layerNames.size(); ++i) {
            if (layerNames[i] == targetLayerName) {
                layerIndex = i;
                break;
            }
        }
        
        if (layerIndex == -1 || layerIndex >= mapLayers.size()) {
            continue; // Skip if layer not found
        }
        
        const QVector<QVector<int>>& layer = mapLayers[layerIndex];
        int tilesRendered = 0;
        
        for (int y = 0; y < layer.size(); ++y) {
            for (int x = 0; x < layer[y].size(); ++x) {
                int tileId = layer[y][x];
                if (tileId == 0) continue; // Skip empty tiles
                
                // Find the correct tileset for this tile ID
                QPixmap tileImage = getTileImage(tileId);
                if (!tileImage.isNull()) {
                    QRect destRect(x * TILE_SIZE - offset.x(), y * TILE_SIZE - offset.y(), TILE_SIZE, TILE_SIZE);
                    painter.drawPixmap(destRect, tileImage);
                    tilesRendered++;
                }
            }
        }

    }
}

QPixmap Level::getTileImage(int tileId)
{
    if (tileId == 0) {
        return QPixmap(); // Empty tile
    }
    
    // First check if this is an individual tile image
    if (individualTileImages.contains(tileId)) {
        return individualTileImages[tileId];
    }
    
    // Find the correct tileset and extract the tile
    int tilesetFirstGid = 0;
    QPixmap tilesetImage;
    
    // Find the tileset that contains this tile ID
    // We need to find the highest firstGid that is <= tileId
    QList<int> sortedGids = tilesetImages.keys();
    std::sort(sortedGids.begin(), sortedGids.end());
    
    for (int i = sortedGids.size() - 1; i >= 0; --i) {
        int firstGid = sortedGids[i];
        if (tileId >= firstGid) {
            tilesetFirstGid = firstGid;
            tilesetImage = tilesetImages[firstGid];
            break;
        }
    }
    
    if (tilesetImage.isNull()) {
        return QPixmap(); // Return empty pixmap if tileset not found
    }
    
    // Calculate tile position within the tileset
    int localTileId = tileId - tilesetFirstGid;
    int tilesPerRow = tilesetImage.width() / TILE_SIZE;
    if (tilesPerRow == 0) {
        return QPixmap(); // Invalid tileset
    }
    
    int tileX = (localTileId % tilesPerRow) * TILE_SIZE;
    int tileY = (localTileId / tilesPerRow) * TILE_SIZE;
    
    // Extract the tile from the tileset
    QRect sourceRect(tileX, tileY, TILE_SIZE, TILE_SIZE);
    return tilesetImage.copy(sourceRect);
}

void Level::loadTMXMap()
{
    // Load TMX map data and parse collision layer
    parseTMXCollisionLayer();
    
    // Load tilesets for rendering
    loadTilesets();
    
    // Parse all visual layers for rendering
    parseTMXVisualLayers();

    // Create some trees
    QPixmap treeSmallSurf = ResourceLoader::loadImage("graphics/objects/tree_small.png");
    QPixmap treeLargeSurf = ResourceLoader::loadImage("graphics/objects/tree_medium.png");

    if (!treeSmallSurf.isNull()) {

        QVector<SpriteGroup*> treeGroups;
        treeGroups.append(allSprites);
        treeGroups.append(collisionSprites);
        treeGroups.append(treeSprites);

        // Create some trees at various positions
        Tree* tree1 = new Tree(QPoint(200, 200), treeSmallSurf, treeGroups, "Small",
                              [this](const QString& item) { playerAdd(item); }, this);
        Tree* tree2 = new Tree(QPoint(400, 300), treeSmallSurf, treeGroups, "Small",
                              [this](const QString& item) { playerAdd(item); }, this);

    } else {

    }

    // Create water tiles
    QVector<QPixmap> waterFrames = ResourceLoader::importFolder("graphics/water");
    if (!waterFrames.isEmpty()) {
        QVector<SpriteGroup*> waterGroups;
        waterGroups.append(allSprites);

        // Create some water tiles
        for (int x = 30; x < 35; ++x) {
            for (int y = 7; y < 12; ++y) {
                Water* water = new Water(QPoint(x * TILE_SIZE, y * TILE_SIZE), waterFrames, waterGroups);
                water->setParent(this);
            }
        }
    }

    // Create player
    player = new Player(QPointF(640, 360), allSprites, collisionSprites, treeSprites,
                       interactionSprites, soilLayer, [this]() { toggleShop(); }, this);
    player->level = this;


    // Create some interaction objects
    QVector<SpriteGroup*> interactionGroups;
    interactionGroups.append(interactionSprites);

    // Trader
    Interaction* trader = new Interaction(QPoint(895, 379), QSize(192, 131), interactionGroups, "Trader");
    trader->setParent(this);

    // Bed
    Interaction* bed = new Interaction(QPoint(1408, 1403), QSize(64, 66), interactionGroups, "Bed");
    bed->setParent(this);
}

void Level::setupAudio()
{
    successSound = new QSoundEffect(this);
    QString successPath = ResourceLoader::getResourcePath("audio/success.wav");
    successSound->setSource(QUrl::fromLocalFile(successPath));
    successSound->setVolume(0.3);

    musicSound = new QSoundEffect(this);
    QString musicPath = ResourceLoader::getResourcePath("audio/music.wav");
    musicSound->setSource(QUrl::fromLocalFile(musicPath));
    musicSound->setVolume(0.5);
    musicSound->setLoopCount(QSoundEffect::Infinite);
    musicSound->play();
}

void Level::run(float dt, QPainter& painter, const QList<int>& pressedKeys)
{
    // Handle intro animation first
    if (introAnimation && introAnimation->isActive()) {
        // Update and display intro animation
        introAnimation->update(dt);
        introAnimation->display(painter);
        
        // Check for skip input (any key pressed)
        if (!pressedKeys.isEmpty()) {
            // Skip animation
            introAnimation->skip();
        }
        
        return; // Don't run normal game logic during intro
    }
    
    // Handle ending animation
    if (endingAnimation && endingAnimation->isActive()) {
        // Update and display ending animation
        endingAnimation->update(dt);
        endingAnimation->display(painter);
        endingAnimation->handleInput(pressedKeys);
        
        return; // Don't run normal game logic during ending
    }
    
    // Game loop running normally

    // Draw sky background first
    if (sky) {
        sky->display(painter, dt);
    }
    
    // Draw map layers
    if (player) {
        QPointF offset;
        offset.setX(player->rect.center().x() - SCREEN_WIDTH / 2.0);
        offset.setY(player->rect.center().y() - SCREEN_HEIGHT / 2.0);
        renderTMXLayers(painter, offset);
    }

    // Draw all sprites
    if (allSprites && player) {
        allSprites->customDraw(painter, player);
    }
    
    // Draw plant sprites separately
    if (soilLayer && soilLayer->plantSprites && player) {
        // Calculate camera offset (same as CameraGroup)
        QPointF offset;
        offset.setX(player->rect.center().x() - SCREEN_WIDTH / 2.0);
        offset.setY(player->rect.center().y() - SCREEN_HEIGHT / 2.0);
        
        // Draw each plant sprite
        for (Sprite* sprite : soilLayer->plantSprites->sprites()) {
            if (sprite && sprite->alive) {
                QRect offsetRect = sprite->rect;
                offsetRect.translate(-offset.x(), -offset.y());
                painter.drawPixmap(offsetRect, sprite->image);
            }
        }
    }

    // Update energy system (always check, regardless of shop state)
    if (player) {
        // Check for game over immediately (energy = 0)
        if (player->energy <= 0) {
            // Calculate survival time
            float totalHours = (currentDay - 1) * 24.0f + currentTime;
            endingAnimation->start(EndingType::FAILURE, currentDay, totalHours);
            return;
        }
        
        // Decrease energy over time
        energyTimer += dt;
        if (energyTimer >= energyDecreaseInterval) {
            player->decreaseEnergy(1);
            energyTimer = 0.0f;
        }
    }
    
    // Handle cheat keys
    for (int key : pressedKeys) {
        if (key == Qt::Key_QuoteLeft) { // ` key
            if (player) {
                player->money += 1000;
            }
        } else if (key == Qt::Key_Backspace) {
            if (player) {
                player->decreaseEnergy(20);
            }
        }
    }

    // Update game objects
    if (shopActive) {
        if (menu) {
            menu->update(dt);
            menu->handleInput(pressedKeys);
            menu->display(painter);
        }
    } else {
        if (allSprites) {
            allSprites->update(dt);
        }
        
        // Update plant sprites separately
        if (soilLayer && soilLayer->plantSprites) {
            soilLayer->plantSprites->update(dt);
        }
         
         // Update time system
         currentTime += timeSpeed * dt;
         if (currentTime >= 24.0f) {
             currentTime = 0.0f;
             currentDay++;
             // Trigger new day - change weather randomly
             isRaining = QRandomGenerator::global()->bounded(10) > 6; // 30% chance of rain
             raining = isRaining;
             if (soilLayer) {
                 soilLayer->raining = raining;
                 // Reset soil water status based on weather
                 soilLayer->removeWater();
                 if (raining) {
                     soilLayer->waterAll();
                 }
             }
         }
         
         plantCollision();
    }

    // Display overlay
    if (!shopActive && overlay) {
        overlay->display(painter);
    }

    // Weather effects
    if (raining && !shopActive && rain) {
        rain->update(dt);
    }

    // Transition overlay
    if (player && player->sleep && transition) {
        transition->play();
    }
}

void Level::playerAdd(const QString& item)
{
    if (player->inventory.contains(item)) {
        player->inventory[item]++;
    } else {
        player->inventory[item] = 1;
    }
    if (successSound) {
        successSound->play();
    }
}

void Level::toggleShop()
{
    shopActive = !shopActive;
}

void Level::reset()
{
    // Update plants
    soilLayer->updatePlants();

    // Reset soil
    soilLayer->removeWater();
    
    // Advance to next day and change weather
    currentDay++;
    currentTime = 6.0f; // Start new day at 6 AM
    isRaining = QRandomGenerator::global()->bounded(10) > 6; // 30% chance of rain
    raining = isRaining;
    soilLayer->raining = raining;
    if (raining) {
        soilLayer->waterAll();
    }

    // Reset apples on trees
    for (Sprite* sprite : treeSprites->sprites()) {
        Tree* tree = qobject_cast<Tree*>(sprite);
        if (tree) {
            // Clear existing apples and create new ones
            tree->createFruit();
        }
    }

    // Reset sky
    sky->startColor = QColor(255, 255, 255);
}

void Level::plantCollision()
{
    if (!soilLayer->plantSprites || soilLayer->plantSprites->empty()) {
        return;
    }

    for (Sprite* sprite : soilLayer->plantSprites->sprites()) {
        // Check if plant is harvestable and collides with player
        Plant* plant = qobject_cast<Plant*>(sprite);
        if (plant && plant->harvestable && sprite->rect.intersects(player->hitbox)) {
            // Remove plant from soil grid
            QPoint gridPos = soilLayer->worldToGrid(plant->getWorldPosition());
            if (soilLayer->isValidGridPos(gridPos)) {
                soilLayer->grid[gridPos.y()][gridPos.x()].removeAll("P");
            }
            
            // Harvest plant - add random 2-3 items to inventory
            int harvestAmount = QRandomGenerator::global()->bounded(2, 4); // 2-3 items
            for (int i = 0; i < harvestAmount; i++) {
                playerAdd(plant->plantType);
            }
            
            // Harvesting consumes 1 energy
            player->decreaseEnergy(1);
            
            sprite->kill();

            // Create particle effect
            QVector<SpriteGroup*> particleGroups;
            particleGroups.append(allSprites);
            Particle* particle = new Particle(sprite->rect.topLeft(), sprite->image, particleGroups, MAIN);
            particle->setParent(this);

            break;
        }
    }
}
