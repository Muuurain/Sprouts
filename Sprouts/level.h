#ifndef LEVEL_H
#define LEVEL_H

#include <QObject>
#include <QPainter>
#include <QSoundEffect>
#include <QRandomGenerator>
#include "gamesettings.h"

class Player;
class CameraGroup;
class SpriteGroup;
class SoilLayer;
class Overlay;
class Transition;
class Rain;
class Sky;
class Menu;
class IntroAnimation;
class EndingAnimation;

class Level : public QObject
{
    Q_OBJECT

public:
    explicit Level(QObject *parent = nullptr);
    ~Level();
    
    // Main game loop
    void run(float dt, QPainter& painter, const QList<int>& pressedKeys = QList<int>());
    
    // Setup
    void setup();
    
    // Game actions
    void playerAdd(const QString& item);
    void toggleShop();
    void reset();
    void plantCollision();
    
    // Properties
    bool shopActive;
    bool raining;

    // Time and weather system (public access for UI)
    int currentDay;
    float currentTime; // Time in hours (0-24)
    float timeSpeed; // How fast time passes (hours per real second)
    bool isRaining;

    // Core components
    Player* player;
    
private:
    CameraGroup* allSprites;
    SpriteGroup* collisionSprites;
    SpriteGroup* treeSprites;
    SpriteGroup* interactionSprites;
    
    // Game systems
    SoilLayer* soilLayer;
    Overlay* overlay;
    Transition* transition;
    Rain* rain;
    Sky* sky;
    Menu* menu;
    IntroAnimation* introAnimation;
    EndingAnimation* endingAnimation;
    
    // Audio
    QSoundEffect* successSound;
    QSoundEffect* musicSound;
    
    // Energy system
    float energyTimer;
    float energyDecreaseInterval; // Time in seconds between energy decreases
    
    // Player spawn position
    QPointF playerSpawnPos;
    
    // Methods
    void loadTMXMap();
    void parseTMXCollisionLayer();
    void parseTMXVisualLayers();
    void renderTMXLayers(QPainter& painter, const QPointF& offset);
    void loadTilesets();
    QPixmap getTileImage(int tileId);
    void setupAudio();
    
    // Map rendering data
    QMap<int, QPixmap> tilesetImages;
    QMap<int, QPixmap> individualTileImages; // For tilesets with individual tile images
    QVector<QVector<QVector<int>>> mapLayers; // [layer][y][x]
    QStringList layerNames;
};

#endif // LEVEL_H