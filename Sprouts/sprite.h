#ifndef SPRITE_H
#define SPRITE_H

#include <QObject>
#include <QPixmap>
#include <QRect>
#include <QRectF>
#include <QVector>
#include <QTimer>
#include "gamesettings.h"

class SpriteGroup;

class Sprite : public QObject
{
    Q_OBJECT

public:
    explicit Sprite(QObject *parent = nullptr);
    virtual ~Sprite();

    // Core properties
    QPixmap image;
    QRect rect;
    QRect hitbox;
    Layer z;
    bool alive;

    // Groups management
    void addToGroup(SpriteGroup* group);
    void removeFromGroup(SpriteGroup* group);
    void kill();

    // Virtual methods
    virtual void update(float /*dt*/) {}
    virtual void animate(float /*dt*/) {}

protected:
    QVector<SpriteGroup*> groups;
};

class Generic : public Sprite
{
    Q_OBJECT

public:
    Generic(const QPoint& pos, const QPixmap& surf, QVector<SpriteGroup*> groups, Layer layer = MAIN);
};

class Interaction : public Generic
{
    Q_OBJECT

public:
    Interaction(const QPoint& pos, const QSize& size, QVector<SpriteGroup*> groups, const QString& name);
    
    QString name;
};

class Water : public Generic
{
    Q_OBJECT

public:
    Water(const QPoint& pos, const QVector<QPixmap>& frames, QVector<SpriteGroup*> groups);
    
    void animate(float dt) override;
    void update(float dt) override;

private:
    QVector<QPixmap> frames;
    float frameIndex;
};

class WildFlower : public Generic
{
    Q_OBJECT

public:
    WildFlower(const QPoint& pos, const QPixmap& surf, QVector<SpriteGroup*> groups);
};

class Particle : public Generic
{
    Q_OBJECT

public:
    Particle(const QPoint& pos, const QPixmap& surf, QVector<SpriteGroup*> groups, Layer layer, int duration = 200);
    
    void update(float dt) override;

private:
    QTimer* timer;
    int duration;
    qint64 startTime;
};

#endif // SPRITE_H