#include "menu.h"
#include "player.h"
#include "gamesettings.h"
#include <QPainter>
#include <QKeyEvent>
#include <QtMath>

Menu::Menu(Player* player, std::function<void()> toggleMenu, std::function<void()> triggerSuccessEnding, QObject *parent)
    : QObject{parent}, player(player), toggleMenu(toggleMenu), triggerSuccessEnding(triggerSuccessEnding)
{
    font = QFont("Arial", 16, QFont::Bold);
    
    // Menu layout
    width = 400;
    height = 500;
    space = 10;
    paddingX = 20;
    paddingY = 20;
    
    // Menu options
    options = {
        {"buy", "corn seeds", 2, 4},
        {"buy", "tomato seeds", 3, 5},
        {"buy", "food", 1, 20},
        {"buy", "bower boat", 1, 9999},
        {"sell", "wood", 2, 4},
        {"sell", "apple", 1, 2},
        {"sell", "corn", 1, 3},
        {"sell", "tomato", 2, 4}
    };
    
    sellBorder = 5; // First 5 are buy options (including food and bower boat)
    setupMenuOptions();
    
    index = 0;
    
    // Setup input timer
    timer = new GameTimer(200, nullptr, this); // 200ms cooldown
}

void Menu::setupMenuOptions()
{
    // Calculate menu position (center of screen)
    int menuX = (SCREEN_WIDTH - width) / 2;
    int menuY = (SCREEN_HEIGHT - height) / 2;
    
    // Calculate text height
    QFontMetrics metrics(font);
    textHeight = metrics.height();
    
    // Calculate total height needed
    int totalHeight = options.size() * (textHeight + space) + 2 * paddingY;
    
    // Adjust menu height if needed
    if (totalHeight > height) {
        height = totalHeight;
        menuY = (SCREEN_HEIGHT - height) / 2;
    }
    
    menuRect = QRect(menuX, menuY, width, height);
}

void Menu::display(QPainter& painter)
{
    // Draw menu background
    painter.fillRect(menuRect, QColor(0, 0, 0, 180));
    painter.setPen(QPen(Qt::white, 2));
    painter.drawRect(menuRect);
    
    // Display money
    displayMoney(painter);
    
    // Display menu entries
    for (int i = 0; i < options.size(); ++i) {
        displayEntry(painter, i);
    }
}

void Menu::displayMoney(QPainter& painter)
{
    painter.setPen(Qt::white);
    painter.setFont(font);
    
    QString moneyText = QString("Money: $%1").arg(player->money);
    QRect textRect(menuRect.x() + paddingX, menuRect.y() + paddingY, 
                   menuRect.width() - 2 * paddingX, textHeight);
    painter.drawText(textRect, Qt::AlignCenter, moneyText);
}

void Menu::displayEntry(QPainter& painter, int index)
{
    const MenuOption& option = options[index];
    
    // Calculate position
    int y = menuRect.y() + paddingY + textHeight + space + index * (textHeight + space);
    QRect entryRect(menuRect.x() + paddingX, y, menuRect.width() - 2 * paddingX, textHeight);
    
    // Highlight current selection
    if (index == this->index) {
        painter.fillRect(entryRect, QColor(255, 255, 255, 50));
    }
    
    // Set text color based on affordability/availability
    bool canAfford = true;
    bool hasItem = true;
    
    if (option.text == "buy") {
        canAfford = (player->money >= option.price);
        painter.setPen(canAfford ? Qt::white : Qt::red);
    } else {
        hasItem = (player->inventory.value(option.item, 0) >= option.amount);
        painter.setPen(hasItem ? Qt::white : Qt::red);
    }
    
    // Create display text
    QString displayText;
    if (option.text == "buy") {
        displayText = QString("Buy %1 %2 - $%3")
                     .arg(option.amount)
                     .arg(option.item)
                     .arg(option.price);
    } else {
        displayText = QString("Sell %1 %2 - $%3")
                     .arg(option.amount)
                     .arg(option.item)
                     .arg(option.price);
    }
    
    painter.setFont(font);
    painter.drawText(entryRect, Qt::AlignLeft | Qt::AlignVCenter, displayText);
}

void Menu::update(float dt)
{
    timer->update();
}

void Menu::handleInput(const QList<int>& pressedKeys)
{
    if (timer->isActive()) return;
    
    // Navigate menu
    if (pressedKeys.contains(Qt::Key_Up) || pressedKeys.contains(Qt::Key_W)) {
        index = (index - 1 + options.size()) % options.size();
        timer->activate();
    }
    else if (pressedKeys.contains(Qt::Key_Down) || pressedKeys.contains(Qt::Key_S)) {
        index = (index + 1) % options.size();
        timer->activate();
    }
    else if (pressedKeys.contains(Qt::Key_Space)) {
        // Execute current option
        executeCurrentOption();
        timer->activate();
    }
    else if (pressedKeys.contains(Qt::Key_Escape)) {
        // Close menu
        if (toggleMenu) {
            toggleMenu();
        }
        timer->activate();
    }
}

void Menu::executeCurrentOption()
{
    if (index < 0 || index >= options.size()) return;
    
    const MenuOption& option = options[index];
    
    if (option.text == "buy") {
        // Buy item
        if (player->money >= option.price) {
            player->money -= option.price;
            
            if (option.item == "corn seeds") {
                player->seedInventory["corn"] += option.amount;
            } else if (option.item == "tomato seeds") {
                player->seedInventory["tomato"] += option.amount;
            } else if (option.item == "food") {
                // Restore full energy when buying food
                player->restoreEnergy();
            } else if (option.item == "bower boat") {
                // Trigger success ending when bower boat is purchased
                if (triggerSuccessEnding) {
                    triggerSuccessEnding();
                }
            } else {
                player->inventory[option.item] += option.amount;
            }
        }
    } else {
        // Sell item
        if (player->inventory.value(option.item, 0) >= option.amount) {
            player->inventory[option.item] -= option.amount;
            player->money += option.price;
            
            // Remove item if count reaches 0
            if (player->inventory[option.item] <= 0) {
                player->inventory.remove(option.item);
            }
        }
    }
}