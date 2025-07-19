#ifndef TRANSITION_H
#define TRANSITION_H

#include <QObject>
#include <QPainter>
#include <QColor>
#include <functional>
#include "gamesettings.h"

class Player;

class Transition : public QObject
{
    Q_OBJECT

public:
    explicit Transition(std::function<void()> reset, Player* player, QObject *parent = nullptr);
    
    // Play transition effect
    void play();
    void update(float dt);
    void display(QPainter& painter);
    
private:
    std::function<void()> reset;
    Player* player;
    
    // Transition properties
    QColor color;
    int speed;
    bool active;
    
    // Helper methods
    void updateTransition();
};

#endif // TRANSITION_H