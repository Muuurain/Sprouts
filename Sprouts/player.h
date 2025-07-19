#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QPixmap>
#include <QRect>
#include <QPointF>
#include <QMap>
#include <QVector>
#include <QKeyEvent>
#include <QSoundEffect>
#include <functional>
#include "sprite.h"
#include "gametimer.h"
#include "gamesettings.h"
#include "soillayer.h"

class SpriteGroup;
class Level;

class Player : public Sprite
{
    Q_OBJECT

public:
    explicit Player(const QPointF& pos, SpriteGroup* group, 
                   SpriteGroup* collisionSprites, SpriteGroup* treeSprites,
                   SpriteGroup* interactionSprites, SoilLayer* soilLayer,
                   std::function<void()> toggleShop, QObject *parent = nullptr);

    // Core methods
    void update(float dt) override;
    void animate(float dt) override;
    void handleInput(const QList<int>& pressedKeys);
    
    // Movement
    void move(float dt);
    void collision(const QString& direction);
    
    // Tools and seeds
    void useTool();
    void useSeed();
    void getTargetPos();
    
    // Status management
    void getStatus();
    void updateTimers();
    
    // Asset loading
    void importAssets();
    
    // Energy management
    void restoreEnergy();
    void decreaseEnergy(int amount = 1);
    
    // Public properties
    QString status;
    float frameIndex;
    QPointF direction;
    QPointF pos;
    float speed;
    QPointF targetPos;
    
    // Tools and inventory
    QVector<QString> tools;
    int toolIndex;
    QString selectedTool;
    
    QVector<QString> seeds;
    int seedIndex;
    QString selectedSeed;
    
    QMap<QString, int> inventory;
    QMap<QString, int> seedInventory;
    int money;
    
    // Energy system
    int energy;
    int maxEnergy;
    
    // Game state
    bool sleep;
    
    // Level reference
    Level* level;
    
signals:
    void playerAddItem(const QString& item);
    
private:
    // Animation frames
    QMap<QString, QVector<QPixmap>> animations;
    
    // Sprite groups
    SpriteGroup* collisionSprites;
    SpriteGroup* treeSprites;
    SpriteGroup* interactionSprites;
    SoilLayer* soilLayer;
    
    // Timers
    QMap<QString, GameTimer*> timers;
    
    // Callbacks
    std::function<void()> toggleShop;
    
    // Sound effects
    QSoundEffect* wateringSound;
    
    // Private helper methods
    QString getDirectionFromStatus() const;
    void setupTimers();
    void checkInitialPosition();
};

#endif // PLAYER_H