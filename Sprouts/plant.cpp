#include "plant.h"
#include "spritegroup.h"
#include "resourceloader.h"
#include "gamesettings.h"
#include <QDebug>
#include <QDir>

Plant::Plant(const QString& plantType, QVector<SpriteGroup*> groups, 
             Sprite* soil, std::function<bool(const QPointF&)> checkWatered, 
             QObject *parent)
    : Sprite(parent), plantType(plantType), soil(soil), checkWatered(checkWatered),
      age(0), harvestable(false)
{
    // Load all growth frames
    loadFrames();
    
    if (frames.isEmpty()) {
        qDebug() << "Plant: No frames loaded for" << plantType;
        return;
    }
    
    maxAge = frames.size() - 1;
    
    // Set growth speed based on plant type
    if (plantType == "corn") {
        growSpeed = 0.01f; // Slower growth
        yOffset = -16;
    } else if (plantType == "tomato") {
        growSpeed = 0.015f; // Faster growth
        yOffset = -8;
    } else {
        growSpeed = 0.01f;
        yOffset = -8;
    }
    
    // Initial setup
    image = frames[0];
    z = GROUND_PLANT;
    
    // Position relative to soil center
    if (soil) {
        QPoint soilCenter = soil->rect.center();
        rect = QRect(soilCenter + QPoint(-image.width() / 2, -image.height() / 2 + yOffset), image.size());
    }
    
    // Add to groups
    for (SpriteGroup* group : groups) {
        addToGroup(group);
    }
    
    qDebug() << "Plant: Created" << plantType << "at" << rect.topLeft() << "with" << frames.size() << "frames";
}

void Plant::loadFrames()
{
    QString basePath = QString("graphics/fruit/%1").arg(plantType);
    
    // Try to load frames 0.png, 1.png, 2.png, 3.png
    for (int i = 0; i < 4; ++i) {
        QString framePath = QString("%1/%2.png").arg(basePath).arg(i);
        QPixmap frame = ResourceLoader::loadImage(framePath);
        
        if (!frame.isNull()) {
            frames.append(frame);
        } else {
            qDebug() << "Plant: Failed to load frame" << framePath;
            break; // Stop loading if a frame is missing
        }
    }
}

void Plant::grow()
{
    if (!checkWatered || !checkWatered(rect.center())) {
        return; // Can't grow without water
    }
    
    age += growSpeed;
    
    // Change layer when plant starts growing
    if (int(age) > 0 && z == GROUND_PLANT) {
        z = MAIN;
        // Create hitbox for collision
        int hitboxWidth = rect.width() * 0.6;
        int hitboxHeight = rect.height() * 0.4;
        hitbox = QRect(rect.x() + (rect.width() - hitboxWidth) / 2,
                      rect.y() + rect.height() - hitboxHeight,
                      hitboxWidth, hitboxHeight);
    }
    
    // Cap age at maximum
    if (age >= maxAge) {
        age = maxAge;
        harvestable = true;
    }
    
    updateImage();
}

void Plant::updateImage()
{
    int frameIndex = qMin(int(age), maxAge);
    if (frameIndex >= 0 && frameIndex < frames.size()) {
        image = frames[frameIndex];
        
        // Update position relative to soil center
        if (soil) {
            QPoint soilCenter = soil->rect.center();
            rect = QRect(soilCenter + QPoint(-image.width() / 2, -image.height() / 2 + yOffset), image.size());
        }
    }
}

void Plant::update(float dt)
{
    if (alive) {
        grow();
    }
}