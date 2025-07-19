#ifndef TREE_H
#define TREE_H

#include <QObject>
#include <QPixmap>
#include <QSoundEffect>
#include <functional>
#include "sprite.h"
#include "spritegroup.h"
#include "gamesettings.h"

class Tree : public Generic
{
    Q_OBJECT

public:
    Tree(const QPoint& pos, const QPixmap& surf, QVector<SpriteGroup*> groups, 
         const QString& name, std::function<void(const QString&)> playerAdd, QObject *parent = nullptr);
    
    // Tree actions
    void damage();
    void checkDeath();
    void createFruit();
    
    // Update
    void update(float dt) override;
    
    // Properties
    int health;
    bool alive;
    QString treeName;
    
private:
    // Callback
    std::function<void(const QString&)> playerAdd;
    
    // Tree resources
    QPixmap stumpSurf;
    QPixmap appleSurf;
    QVector<QPoint> applePos;
    SpriteGroup* appleSprites;
    
    // Sound
    QSoundEffect* axeSound;
};

#endif // TREE_H