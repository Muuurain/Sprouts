#include "sky.h"
#include "gamesettings.h"
#include "resourceloader.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QtMath>

// Sky implementation
Sky::Sky(QObject *parent)
    : QObject{parent}
{
    startColor = QColor(255, 255, 255); // Day color
    endColor = QColor(38, 101, 189);    // Night color
}

void Sky::display(QPainter& painter, float dt, float currentTime)
{
    // Update colors based on time
    updateColor(currentTime);
    
    // Create gradient from start to end color
    QLinearGradient gradient(0, 0, 0, SCREEN_HEIGHT);
    gradient.setColorAt(0, startColor);
    gradient.setColorAt(1, endColor);
    
    painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, gradient);
}

void Sky::updateColor(float currentTime)
{
    // Normalize time to 0-24 range
    float normalizedTime = fmod(currentTime, 24.0f);
    
    // Define time periods
    // 6-18: Day (white to light blue)
    // 18-22: Evening (orange to dark blue)
    // 22-6: Night (dark blue to black)
    
    if (normalizedTime >= 6.0f && normalizedTime < 18.0f) {
        // Daytime: 6 AM to 6 PM
        startColor = QColor(255, 255, 255); // White
        endColor = QColor(135, 206, 235);   // Sky blue
    }
    else if (normalizedTime >= 18.0f && normalizedTime < 22.0f) {
        // Evening: 6 PM to 10 PM
        float eveningProgress = (normalizedTime - 18.0f) / 4.0f; // 0-1
        
        // Interpolate from day colors to night colors
        int startR = 255 - (int)(100 * eveningProgress); // 255 -> 155
        int startG = 255 - (int)(100 * eveningProgress); // 255 -> 155
        int startB = 255 - (int)(155 * eveningProgress); // 255 -> 100
        
        int endR = 135 - (int)(97 * eveningProgress);  // 135 -> 38
        int endG = 206 - (int)(105 * eveningProgress); // 206 -> 101
        int endB = 235 - (int)(46 * eveningProgress);  // 235 -> 189
        
        startColor = QColor(startR, startG, startB);
        endColor = QColor(endR, endG, endB);
    }
    else {
        // Night: 10 PM to 6 AM
        startColor = QColor(25, 25, 112);   // Midnight blue
        endColor = QColor(0, 0, 0);         // Black
    }
}

void Sky::reset()
{
    startColor = QColor(255, 255, 255); // Reset to day
    endColor = QColor(38, 101, 189);
}

// Drop implementation
Drop::Drop(const QPointF& pos, const QPixmap& surf, bool moving, SpriteGroup* group)
    : Generic(pos.toPoint(), surf, QVector<SpriteGroup*>{group}, RAIN), moving(moving)
{
    // Random lifetime for drops
    lifetime = QRandomGenerator::global()->bounded(400, 500);
    startTime = 0;
    
    if (moving) {
        direction = QPointF(QRandomGenerator::global()->bounded(-2, 3), 
                           QRandomGenerator::global()->bounded(4, 7));
        speed = QRandomGenerator::global()->bounded(200, 250);
    }
}

void Drop::update(float dt)
{
    if (moving) {
        // Move the drop
        QPointF movement = direction * speed * dt;
        rect.translate(movement.toPoint());
        hitbox.translate(movement.toPoint());
        
        // Remove if off screen (use map height instead of screen height)
        const int MAP_HEIGHT = 40 * 64; // 40 tiles * 64 pixels per tile = 2560
        if (rect.y() > MAP_HEIGHT) {
            kill();
        }
    } else {
        // Ground drop - fade out over time
        startTime += dt * 1000; // Convert to milliseconds
        if (startTime > lifetime) {
            kill();
        }
    }
}

// Rain implementation
Rain::Rain(SpriteGroup* allSprites, QObject *parent)
    : QObject{parent}, allSprites(allSprites), rainTimer(0.0f), floorTimer(0.0f)
{
    // Load rain graphics
    rainDrops.append(QPixmap(ResourceLoader::getResourcePath("graphics/rain/drops/0.png")));
    rainDrops.append(QPixmap(ResourceLoader::getResourcePath("graphics/rain/drops/1.png")));
    rainDrops.append(QPixmap(ResourceLoader::getResourcePath("graphics/rain/drops/2.png")));
    
    rainFloor.append(QPixmap(ResourceLoader::getResourcePath("graphics/rain/floor/0.png")));
    rainFloor.append(QPixmap(ResourceLoader::getResourcePath("graphics/rain/floor/1.png")));
    rainFloor.append(QPixmap(ResourceLoader::getResourcePath("graphics/rain/floor/2.png")));
    
    // Remove null pixmaps (filter out empty pixmaps)
    for (int i = rainDrops.size() - 1; i >= 0; --i) {
        if (rainDrops[i].isNull()) {
            rainDrops.removeAt(i);
        }
    }
    for (int i = rainFloor.size() - 1; i >= 0; --i) {
        if (rainFloor[i].isNull()) {
            rainFloor.removeAt(i);
        }
    }
}

void Rain::createRainDrops()
{
    if (rainDrops.isEmpty()) return;
    
    // Map dimensions: 50x40 tiles, each tile is 64 pixels
    const int MAP_WIDTH = 50 * TILE_SIZE;  // 3200 pixels
    const int MAP_HEIGHT = 40 * TILE_SIZE; // 2560 pixels
    
    // Create falling rain drops
    for (int i = 0; i < 10; ++i) {
        int x = QRandomGenerator::global()->bounded(MAP_WIDTH);
        int y = QRandomGenerator::global()->bounded(-50, -10);
        
        int surfIndex = QRandomGenerator::global()->bounded(rainDrops.size());
        QPixmap surf = rainDrops[surfIndex];
        
        QVector<SpriteGroup*> groups;
        groups.append(allSprites);
        
        Drop* drop = new Drop(QPointF(x, y), surf, true, allSprites);
        drop->setParent(this);
    }
}

void Rain::createFloorDrops()
{
    if (rainFloor.isEmpty()) return;
    
    // Map dimensions: 50x40 tiles, each tile is 64 pixels
    const int MAP_WIDTH = 50 * TILE_SIZE;  // 3200 pixels
    const int MAP_HEIGHT = 40 * TILE_SIZE; // 2560 pixels
    
    // Create ground rain effects
    for (int i = 0; i < 3; ++i) {
        int x = QRandomGenerator::global()->bounded(MAP_WIDTH);
        int y = QRandomGenerator::global()->bounded(MAP_HEIGHT);
        
        int surfIndex = QRandomGenerator::global()->bounded(rainFloor.size());
        QPixmap surf = rainFloor[surfIndex];
        
        QVector<SpriteGroup*> groups;
        groups.append(allSprites);
        
        Drop* drop = new Drop(QPointF(x, y), surf, false, allSprites);
        drop->setParent(this);
    }
}

void Rain::update(float dt)
{
    rainTimer += dt;
    floorTimer += dt;
    
    // Create rain drops more frequently (every 0.05 seconds)
    if (rainTimer >= 0.05f) {
        createRainDrops();
        rainTimer = 0;
    }
    
    // Create floor drops more frequently (every 0.1 seconds)
    if (floorTimer >= 0.1f) {
        createFloorDrops();
        floorTimer = 0;
    }
}