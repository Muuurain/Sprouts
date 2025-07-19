#ifndef SPRITEGROUP_H
#define SPRITEGROUP_H

#include <QObject>
#include <QVector>
#include <QPainter>
#include <QPointF>
#include "gamesettings.h"

class Sprite;
class Player;

class SpriteGroup : public QObject
{
    Q_OBJECT

public:
    explicit SpriteGroup(QObject *parent = nullptr);
    virtual ~SpriteGroup();

    // Sprite management
    void addSprite(Sprite* sprite);
    void removeSprite(Sprite* sprite);
    QVector<Sprite*> sprites() const { return spriteList; }
    
    // Update all sprites
    virtual void update(float dt);
    
    // Clear all sprites
    void clear();
    
    // Check if empty
    bool empty() const { return spriteList.isEmpty(); }
    
    // Size
    int size() const { return spriteList.size(); }

protected:
    QVector<Sprite*> spriteList;
};

class CameraGroup : public SpriteGroup
{
    Q_OBJECT

public:
    explicit CameraGroup(QObject *parent = nullptr);
    
    // Custom drawing with camera offset
    void customDraw(QPainter& painter, Player* player);
    
    // Camera offset
    QPointF offset;

private:
    void sortSpritesByLayer();
};

#endif // SPRITEGROUP_H