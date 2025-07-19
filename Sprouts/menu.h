#ifndef MENU_H
#define MENU_H

#include <QObject>
#include <QPainter>
#include <QFont>
#include <QVector>
#include <QString>
#include <functional>
#include "gametimer.h"
#include "gamesettings.h"

class Player;

class Menu : public QObject
{
    Q_OBJECT

public:
    explicit Menu(Player* player, std::function<void()> toggleMenu, std::function<void()> triggerSuccessEnding = nullptr, QObject *parent = nullptr);
    
    // Update and display menu
    void update(float dt);
    void display(QPainter& painter);
    void handleInput(const QList<int>& pressedKeys);
    
private:
    Player* player;
    std::function<void()> toggleMenu;
    std::function<void()> triggerSuccessEnding;
    
    // Menu properties
    QFont font;
    int width;
    int height;
    int space;
    int paddingX;
    int paddingY;
    QRect menuRect;
    int textHeight;
    
    // Menu items
    struct MenuOption {
        QString text;
        QString item;
        int amount;
        int price;
    };
    QVector<MenuOption> options;
    int sellBorder;
    int index;
    
    // Timer for input
    GameTimer* timer;
    
    // Helper methods
    void setupMenuOptions();
    void displayMoney(QPainter& painter);
    void displayEntry(QPainter& painter, int index);
    void executeCurrentOption();
};

#endif // MENU_H