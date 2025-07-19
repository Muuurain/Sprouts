#include "spritegroup.h"
#include "sprite.h"
#include "player.h"
#include <algorithm>
#include <QDebug>

// SpriteGroup implementation
SpriteGroup::SpriteGroup(QObject *parent)
    : QObject{parent}
{
}

SpriteGroup::~SpriteGroup()
{
    clear();
}

void SpriteGroup::addSprite(Sprite* sprite)
{
    if (sprite && !spriteList.contains(sprite)) {
        spriteList.append(sprite);
    }
}

void SpriteGroup::removeSprite(Sprite* sprite)
{
    if (sprite) {
        spriteList.removeOne(sprite);
    }
}

void SpriteGroup::update(float dt)
{
    // Update all sprites
    for (Sprite* sprite : spriteList) {
        if (sprite && sprite->alive) {
            sprite->update(dt);
        }
    }
    
    // Remove dead sprites
    spriteList.erase(
        std::remove_if(spriteList.begin(), spriteList.end(),
                      [](Sprite* sprite) { return !sprite || !sprite->alive; }),
        spriteList.end());
}

void SpriteGroup::clear()
{
    for (Sprite* sprite : spriteList) {
        if (sprite) {
            sprite->kill();
        }
    }
    spriteList.clear();
}

// CameraGroup implementation
CameraGroup::CameraGroup(QObject *parent)
    : SpriteGroup{parent}, offset(0, 0)
{
}

void CameraGroup::customDraw(QPainter& painter, Player* player)
{
    if (!player) {
        qDebug() << "CameraGroup: No player found!";
        return;
    }
    
    // Debug: Print sprite count
    static int debugCounter = 0;
    if (debugCounter % 60 == 0) { // Print every 60 frames (~1 second)
        qDebug() << "CameraGroup: Drawing" << spriteList.size() << "sprites";
    }
    debugCounter++;
    
    // Calculate camera offset
    offset.setX(player->rect.center().x() - SCREEN_WIDTH / 2.0);
    offset.setY(player->rect.center().y() - SCREEN_HEIGHT / 2.0);
    
    // Sort sprites by layer and Y position
    sortSpritesByLayer();
    
    // Draw sprites layer by layer
    for (int layer = 0; layer <= RAIN_DROPS; ++layer) {
        // Get sprites for this layer and sort by Y position
        QVector<Sprite*> layerSprites;
        for (Sprite* sprite : spriteList) {
            if (sprite && sprite->alive && sprite->z == layer) {
                layerSprites.append(sprite);
            }
        }
        
        // Sort by Y position for depth
        std::sort(layerSprites.begin(), layerSprites.end(),
                 [](Sprite* a, Sprite* b) {
                     return a->rect.center().y() < b->rect.center().y();
                 });
        
        // Draw sprites in this layer
        for (Sprite* sprite : layerSprites) {
            QRect offsetRect = sprite->rect;
            offsetRect.translate(-offset.x(), -offset.y());
            
            // Only draw if sprite is visible on screen
            if (offsetRect.intersects(QRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT))) {
                painter.drawPixmap(offsetRect.topLeft(), sprite->image);
            }
        }
    }
}

void CameraGroup::sortSpritesByLayer()
{
    std::sort(spriteList.begin(), spriteList.end(),
             [](Sprite* a, Sprite* b) {
                 if (a->z != b->z) {
                     return a->z < b->z;
                 }
                 return a->rect.center().y() < b->rect.center().y();
             });
}