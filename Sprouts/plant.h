#ifndef PLANT_H
#define PLANT_H

#include "sprite.h"
#include <QObject>
#include <QPixmap>
#include <QVector>
#include <QPoint>
#include <QString>
#include <functional>

class SpriteGroup;
class SoilLayer;

class Plant : public Sprite
{
    Q_OBJECT

public:
    explicit Plant(const QString& plantType, QVector<SpriteGroup*> groups, 
                   Sprite* soil, std::function<bool(const QPointF&)> checkWatered, 
                   QObject *parent = nullptr);
    
    // Plant properties
    QString plantType;
    QVector<QPixmap> frames;
    Sprite* soil;
    std::function<bool(const QPointF&)> checkWatered;
    
    // Growth properties
    float age;
    int maxAge;
    float growSpeed;
    bool harvestable;
    int yOffset;
    
    // Methods
    void grow();
    void update(float dt) override;
    QPointF getWorldPosition() const { return rect.center(); }
    
private:
    void loadFrames();
    void updateImage();
};

#endif // PLANT_H