#ifndef SOILLAYER_H
#define SOILLAYER_H

#include <QObject>
#include <QVector>
#include <QPoint>
#include <QPixmap>
#include <QMap>
#include <QSoundEffect>
#include "gamesettings.h"

class SpriteGroup;
class Sprite;
class Plant;

class SoilLayer : public QObject
{
    Q_OBJECT

public:
    explicit SoilLayer(SpriteGroup* allSprites, SpriteGroup* collisionSprites, QObject *parent = nullptr);
    
    // Soil actions
    void getHit(const QPointF& point);
    void water(const QPointF& point);
    bool plantSeed(const QPointF& point, const QString& seed);
    
    // Plant management
    void updatePlants();
    void removeWater();
    void waterAll();
    bool checkWatered(const QPointF& worldPos);
    
    // Properties
    bool raining;
    SpriteGroup* plantSprites;
    
    // Grid system (public for plant harvesting)
    QVector<QVector<QVector<QString>>> grid; // 2D grid with list of states per cell
    QPoint worldToGrid(const QPointF& worldPos);
    QPointF gridToWorld(const QPoint& gridPos);
    bool isValidGridPos(const QPoint& gridPos);
    
private:
    int gridWidth;
    int gridHeight;
    
    // Sprite groups
    SpriteGroup* allSprites;
    SpriteGroup* collisionSprites;
    SpriteGroup* soilSprites;
    SpriteGroup* waterSprites;
    
    // Graphics
    QMap<QString, QPixmap> soilSurfs;
    QMap<QString, QPixmap> waterSurfs;
    
    // Audio
    QSoundEffect* hoeSound;
    QSoundEffect* plantSound;
    
    // Methods
    void createSoilTiles();
    void createWaterTiles();
    void loadSoilGraphics();
    void createSoilGrid();
    void setupAudio();
};

#endif // SOILLAYER_H