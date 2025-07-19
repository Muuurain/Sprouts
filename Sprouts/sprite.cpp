#include "sprite.h"
#include "spritegroup.h"
#include <QDebug>
#include <QDateTime>
#include <QElapsedTimer>

// Sprite implementation
Sprite::Sprite(QObject *parent)
    : QObject{parent}, z(MAIN), alive(true)
{
}

Sprite::~Sprite()
{
    // Remove from all groups
    for (SpriteGroup* group : groups) {
        if (group) {
            group->removeSprite(this);
        }
    }
}

void Sprite::addToGroup(SpriteGroup* group)
{
    if (group && !groups.contains(group)) {
        groups.append(group);
        group->addSprite(this);
    }
}

void Sprite::removeFromGroup(SpriteGroup* group)
{
    if (group && groups.contains(group)) {
        groups.removeOne(group);
        group->removeSprite(this);
    }
}

void Sprite::kill()
{
    alive = false;
    // Remove from all groups
    QVector<SpriteGroup*> groupsCopy = groups; // Copy to avoid modification during iteration
    for (SpriteGroup* group : groupsCopy) {
        removeFromGroup(group);
    }
}

// Generic implementation
Generic::Generic(const QPoint& pos, const QPixmap& surf, QVector<SpriteGroup*> groups, Layer layer)
    : Sprite()
{
    image = surf;
    rect = QRect(pos, surf.size());
    z = layer;
    
    // Create hitbox (smaller than the sprite)
    int hitboxWidth = rect.width() * 0.8;
    int hitboxHeight = rect.height() * 0.25;
    hitbox = QRect(rect.x() + (rect.width() - hitboxWidth) / 2,
                   rect.y() + rect.height() - hitboxHeight,
                   hitboxWidth, hitboxHeight);
    
    // Add to groups
    for (SpriteGroup* group : groups) {
        addToGroup(group);
    }
}

// Interaction implementation
Interaction::Interaction(const QPoint& pos, const QSize& size, QVector<SpriteGroup*> groups, const QString& name)
    : Generic(pos, QPixmap(size), groups), name(name)
{
    // Create a transparent surface for interaction areas
    image = QPixmap(size);
    image.fill(Qt::transparent);
    rect = QRect(pos, size);
}

// Water implementation
Water::Water(const QPoint& pos, const QVector<QPixmap>& frames, QVector<SpriteGroup*> groups)
    : Generic(pos, frames.isEmpty() ? QPixmap() : frames[0], groups, WATER), frames(frames), frameIndex(0)
{
}

void Water::animate(float dt)
{
    if (frames.isEmpty()) return;
    
    frameIndex += 5.0f * dt;
    if (frameIndex >= frames.size()) {
        frameIndex = 0;
    }
    image = frames[static_cast<int>(frameIndex)];
}

void Water::update(float dt)
{
    animate(dt);
}

// WildFlower implementation
WildFlower::WildFlower(const QPoint& pos, const QPixmap& surf, QVector<SpriteGroup*> groups)
    : Generic(pos, surf, groups)
{
    // Adjust hitbox for wildflowers
    int hitboxWidth = rect.width() - 20;
    int hitboxHeight = rect.height() * 0.1;
    hitbox = QRect(rect.x() + 10,
                   rect.y() + rect.height() - hitboxHeight,
                   hitboxWidth, hitboxHeight);
}

// Particle implementation
Particle::Particle(const QPoint& pos, const QPixmap& surf, QVector<SpriteGroup*> groups, Layer layer, int duration)
    : Generic(pos, surf, groups, layer), duration(duration)
{
    startTime = QDateTime::currentMSecsSinceEpoch();
    
    // Create white surface effect
    QPixmap maskedSurf = surf;
    // Apply white color effect (simplified)
    // In a full implementation, you might want to use QPainter to create a mask effect
    image = maskedSurf;
}

void Particle::update(float /*dt*/)
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (currentTime - startTime > duration) {
        kill();
    }
}