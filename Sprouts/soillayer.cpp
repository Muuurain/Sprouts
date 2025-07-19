#include "soillayer.h"
#include "spritegroup.h"
#include "sprite.h"
#include "plant.h"
#include "resourceloader.h"
#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>
#include <QStringList>
#include <QCoreApplication>
#include <QDir>
#include <QUrl>

SoilLayer::SoilLayer(SpriteGroup* allSprites, SpriteGroup* collisionSprites, QObject *parent)
    : QObject{parent}, raining(false), allSprites(allSprites), collisionSprites(collisionSprites)
{
    // Initialize grid
    gridWidth = 50; // Map width in tiles
    gridHeight = 40; // Map height in tiles
    
    grid.resize(gridHeight);
    for (int y = 0; y < gridHeight; ++y) {
        grid[y].resize(gridWidth);
    }
    
    // Initialize plant sprites group
    plantSprites = new SpriteGroup(this);
    
    // Initialize soil sprites group
    soilSprites = new SpriteGroup(this);
    
    // Initialize water sprites group
    waterSprites = new SpriteGroup(this);
    
    // Load soil graphics
    loadSoilGraphics();
    
    // Setup audio
    setupAudio();
    
    // Initialize farmable grid
    createSoilGrid();
}

void SoilLayer::loadSoilGraphics()
{
    // Load soil surfaces
    QStringList soilTypes = {"b", "bl", "bm", "br", "l", "lm", "lr", "lrb", "lrt", 
                            "o", "r", "rm", "soil", "t", "tb", "tbl", "tbr", 
                            "tl", "tm", "tr", "x"};
    
    for (const QString& soilType : soilTypes) {
        QString path = QString("graphics/soil/%1.png").arg(soilType);
        soilSurfs[soilType] = ResourceLoader::loadImage(path);
    }
    
    // Load water surfaces
    for (int i = 0; i < 3; ++i) {
        QString path = QString("graphics/soil_water/%1.png").arg(i);
        waterSurfs[QString::number(i)] = ResourceLoader::loadImage(path);
    }
}

bool SoilLayer::checkWatered(const QPointF& worldPos)
{
    QPoint gridPos = worldToGrid(worldPos);
    if (!isValidGridPos(gridPos)) return false;
    
    return grid[gridPos.y()][gridPos.x()].contains("W");
}

void SoilLayer::getHit(const QPointF& point)
{
    QPoint gridPos = worldToGrid(point);
    if (!isValidGridPos(gridPos)) return;
    
    // Check if this position can be farmed and add tilled soil
    if (grid[gridPos.y()][gridPos.x()].contains("F") && !grid[gridPos.y()][gridPos.x()].contains("X")) {
        // Play hoe sound
        if (hoeSound) {
            hoeSound->play();
        }
        
        grid[gridPos.y()][gridPos.x()].append("X");
        createSoilTiles();
        if (raining) {
            waterAll();
        }
    }
}

void SoilLayer::water(const QPointF& point)
{
    QPoint gridPos = worldToGrid(point);
    if (!isValidGridPos(gridPos)) return;
    
    // Add water to tilled soil
    if (grid[gridPos.y()][gridPos.x()].contains("X") && 
        !grid[gridPos.y()][gridPos.x()].contains("W")) {
        grid[gridPos.y()][gridPos.x()].append("W");
        createWaterTiles();
    }
}

bool SoilLayer::plantSeed(const QPointF& point, const QString& seed)
{
    QPoint gridPos = worldToGrid(point);
    if (!isValidGridPos(gridPos)) return false;
    
    // Plant seed on tilled, watered soil
    if (grid[gridPos.y()][gridPos.x()].contains("X") && 
        grid[gridPos.y()][gridPos.x()].contains("W") &&
        !grid[gridPos.y()][gridPos.x()].contains("P")) {
        
        // Play plant sound
        if (plantSound) {
            plantSound->play();
        }
        
        grid[gridPos.y()][gridPos.x()].append("P");
        
        // Create plant sprite using Plant class
        QVector<SpriteGroup*> plantGroups;
        plantGroups.append(plantSprites);
        
        QPointF worldPos = gridToWorld(gridPos);
        
        // Find the soil sprite at this position for reference
        Sprite* soilSprite = nullptr;
        for (Sprite* sprite : soilSprites->sprites()) {
            if (sprite->rect.contains(worldPos.toPoint())) {
                soilSprite = sprite;
                break;
            }
        }
        
        // Create plant with growth logic
        Plant* plant = new Plant(seed, plantGroups, soilSprite, 
                                [this](const QPointF& pos) { return checkWatered(pos); }, this);
        
        qDebug() << "SoilLayer: Plant created at" << worldPos << "for seed" << seed;
        return true; // Successfully planted
    }
    
    return false; // Failed to plant
}

void SoilLayer::createSoilTiles()
{
    // Clear existing soil sprites
    soilSprites->clear();
    
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            if (grid[y][x].contains("X")) {
                // Check neighboring tiles for tile type selection
                bool t = (y > 0) && grid[y-1][x].contains("X");
                bool b = (y < gridHeight-1) && grid[y+1][x].contains("X");
                bool r = (x < gridWidth-1) && grid[y][x+1].contains("X");
                bool l = (x > 0) && grid[y][x-1].contains("X");
                
                QString tileType = "o";
                
                // All sides
                if (t && r && b && l) tileType = "x";
                
                // Horizontal tiles only
                else if (l && !t && !r && !b) tileType = "r";
                else if (r && !t && !l && !b) tileType = "l";
                else if (r && l && !t && !b) tileType = "lr";
                
                // Vertical only
                else if (t && !r && !l && !b) tileType = "b";
                else if (b && !r && !l && !t) tileType = "t";
                else if (b && t && !r && !l) tileType = "tb";
                
                // Corners
                else if (l && b && !t && !r) tileType = "tr";
                else if (r && b && !t && !l) tileType = "tl";
                else if (l && t && !b && !r) tileType = "br";
                else if (r && t && !b && !l) tileType = "bl";
                
                // T shapes
                else if (t && b && r && !l) tileType = "tbr";
                else if (t && b && l && !r) tileType = "tbl";
                else if (l && r && t && !b) tileType = "lrb";
                else if (l && r && b && !t) tileType = "lrt";
                
                QPointF worldPos = gridToWorld(QPoint(x, y));
                QPixmap soilSurf = soilSurfs[tileType];
                if (!soilSurf.isNull()) {
                    QVector<SpriteGroup*> soilGroups;
                    soilGroups.append(allSprites);
                    soilGroups.append(soilSprites);
                    
                    Generic* soil = new Generic(worldPos.toPoint(), soilSurf, soilGroups, SOIL);
                    soil->setParent(this);
                }
            }
        }
    }
}

void SoilLayer::createWaterTiles()
{
    // Clear existing water sprites
    waterSprites->clear();
    
    // Simplified water tile creation
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            if (grid[y][x].contains("W")) {
                QPointF worldPos = gridToWorld(QPoint(x, y));
                
                // Use basic water texture
                QPixmap waterSurf = waterSurfs["0"];
                if (!waterSurf.isNull()) {
                    QVector<SpriteGroup*> waterGroups;
                    waterGroups.append(allSprites);
                    waterGroups.append(waterSprites);
                    
                    Generic* water = new Generic(worldPos.toPoint(), waterSurf, waterGroups, SOIL_WATER);
                    water->setParent(this);
                }
            }
        }
    }
}

void SoilLayer::updatePlants()
{
    // Update plant growth (simplified)
    if (plantSprites) {
        plantSprites->update(0.016f); // Assume 60 FPS
    }
}

void SoilLayer::removeWater()
{
    // Remove water from all tiles
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            grid[y][x].removeAll("W");
        }
    }
    // Update water tiles to reflect the changes
    createWaterTiles();
}

void SoilLayer::waterAll()
{
    // Add water to all tilled tiles
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            if (grid[y][x].contains("X") && !grid[y][x].contains("W")) {
                grid[y][x].append("W");
            }
        }
    }
    createWaterTiles();
}

QPoint SoilLayer::worldToGrid(const QPointF& worldPos)
{
    return QPoint(static_cast<int>(worldPos.x()) / TILE_SIZE,
                  static_cast<int>(worldPos.y()) / TILE_SIZE);
}

QPointF SoilLayer::gridToWorld(const QPoint& gridPos)
{
    return QPointF(gridPos.x() * TILE_SIZE, gridPos.y() * TILE_SIZE);
}

bool SoilLayer::isValidGridPos(const QPoint& gridPos)
{
    return gridPos.x() >= 0 && gridPos.x() < gridWidth && 
           gridPos.y() >= 0 && gridPos.y() < gridHeight;
}

void SoilLayer::createSoilGrid()
{
    // Parse TMX file to get Farmable layer data
    QString tmxFilePath = "data/map.tmx";
    QFile file(tmxFilePath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open TMX file for farmable data:" << tmxFilePath;
        // Try alternative paths
        QStringList alternativePaths = {
            QCoreApplication::applicationDirPath() + "/data/map.tmx",
            QCoreApplication::applicationDirPath() + "/../data/map.tmx",
            QCoreApplication::applicationDirPath() + "/../../data/map.tmx",
            QCoreApplication::applicationDirPath() + "/../../../data/map.tmx"
        };
        
        bool found = false;
        for (const QString& altPath : alternativePaths) {
            QFile altFile(altPath);
            if (altFile.exists()) {
                tmxFilePath = altPath;
                file.setFileName(tmxFilePath);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    found = true;
                    break;
                }
            }
        }
        
        if (!found) {
            qDebug() << "Could not find TMX file, using fallback farmable area";
            // Fallback to hardcoded area
            for (int y = 15; y < 25 && y < gridHeight; ++y) {
                for (int x = 15; x < 35 && x < gridWidth; ++x) {
                    grid[y][x].append("F");
                }
            }
            return;
        }
    }
    
    QXmlStreamReader xml(&file);
    int farmableCount = 0;
    
    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        
        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == QLatin1String("layer")) {
                QXmlStreamAttributes attributes = xml.attributes();
                QString layerName = attributes.value("name").toString();
                
                if (layerName == "Farmable") {
                    // Find the data element for this layer
                    while (!xml.atEnd() && !(xml.isStartElement() && xml.name() == QLatin1String("data"))) {
                        xml.readNext();
                    }
                    
                    if (xml.isStartElement() && xml.name() == QLatin1String("data")) {
                        QString csvData = xml.readElementText().trimmed();
                        QStringList lines = csvData.split('\n', Qt::SkipEmptyParts);
                        
                        for (int y = 0; y < lines.size() && y < gridHeight; ++y) {
                            QStringList values = lines[y].split(',', Qt::SkipEmptyParts);
                            for (int x = 0; x < values.size() && x < gridWidth; ++x) {
                                int tileId = values[x].trimmed().toInt();
                                if (tileId == 169) { // Farmable tile ID
                                    grid[y][x].append("F");
                                    farmableCount++;
                                }
                            }
                        }
                    }
                    break; // Found Farmable layer, no need to continue
                }
            }
        }
    }
    
    file.close();
    qDebug() << "SoilLayer: Created farmable grid with" << farmableCount << "farmable tiles from TMX data";
}

void SoilLayer::setupAudio()
{
    // Setup hoe sound
    hoeSound = new QSoundEffect(this);
    QString hoePath = ResourceLoader::getResourcePath("audio/hoe.wav");
    hoeSound->setSource(QUrl::fromLocalFile(hoePath));
    hoeSound->setVolume(0.1);
    
    // Setup plant sound
    plantSound = new QSoundEffect(this);
    QString plantPath = ResourceLoader::getResourcePath("audio/plant.wav");
    plantSound->setSource(QUrl::fromLocalFile(plantPath));
    plantSound->setVolume(0.2);
}