#ifndef SKY_H
#define SKY_H

#include <QObject>
#include <QPainter>
#include <QColor>
#include <QVector>
#include <QPixmap>
#include <QPointF>
#include "sprite.h"
#include "gamesettings.h"

class SpriteGroup;

class Sky : public QObject
{
    Q_OBJECT

public:
    explicit Sky(QObject *parent = nullptr);
    
    // Display sky
    void display(QPainter& painter, float dt, float currentTime = 12.0f);
    void reset();
    
    // Properties
    QColor startColor;
    QColor endColor;
    
private:
    void updateColor(float currentTime);
};

class Drop : public Generic
{
    Q_OBJECT

public:
    Drop(const QPointF& pos, const QPixmap& surf, bool moving, SpriteGroup* group);
    
    void update(float dt) override;
    
private:
    int lifetime;
    qint64 startTime;
    bool moving;
    QPointF direction;
    float speed;
};

class Rain : public QObject
{
    Q_OBJECT

public:
    explicit Rain(SpriteGroup* allSprites, QObject *parent = nullptr);
    
    // Update rain
    void update(float dt);
    
private:
    SpriteGroup* allSprites;
    QVector<QPixmap> rainDrops;
    QVector<QPixmap> rainFloor;
    int floorWidth;
    int floorHeight;
    
    // Create rain effects
    void createFloorDrops();
    void createRainDrops();
    
    // Timers
    float rainTimer;
    float floorTimer;
};

#endif // SKY_H