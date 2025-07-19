#include "overlay.h"
#include "player.h"
#include "level.h"
#include "resourceloader.h"
#include "gamesettings.h"
#include <QPainter>
#include <QDebug>

Overlay::Overlay(Player* player, QObject *parent)
    : QObject{parent}, player(player)
{
    loadGraphics();
}

void Overlay::loadGraphics()
{
    // Load tool graphics
    QStringList toolNames = {"hoe", "axe", "water"};
    for (const QString& tool : toolNames) {
        QString path = QString("graphics/overlay/%1.png").arg(tool);
        QPixmap toolPixmap = ResourceLoader::loadImage(path);
        if (!toolPixmap.isNull()) {
            toolSurfs[tool] = toolPixmap;
        }
    }
    
    // Load seed graphics
    QStringList seedNames = {"corn", "tomato"};
    for (const QString& seed : seedNames) {
        QString path = QString("graphics/overlay/%1.png").arg(seed);
        QPixmap seedPixmap = ResourceLoader::loadImage(path);
        if (!seedPixmap.isNull()) {
            seedSurfs[seed] = seedPixmap;
        }
    }
}

void Overlay::display(QPainter& painter)
{
    displayTools(painter);
    displaySeeds(painter);
    displayInventory(painter);
    displayMoney(painter);
    displayEnergy(painter);
    displayTime(painter);
    displayWeather(painter);
}

void Overlay::displayTools(QPainter& painter)
{
    if (!player) return;
    
    // Get current tool
    QString currentTool = player->selectedTool;
    
    // Tool overlay positions
    QPoint toolPos(40, 40);
    int spacing = 80;
    
    QStringList tools = {"hoe", "axe", "water"};
    
    for (int i = 0; i < tools.size(); ++i) {
        const QString& tool = tools[i];
        QPoint pos = toolPos + QPoint(i * spacing, 0);
        
        // Draw tool background
        QRect bgRect(pos.x() - 5, pos.y() - 5, 70, 70);
        if (tool == currentTool) {
            painter.fillRect(bgRect, QColor(255, 255, 255, 100));
        } else {
            painter.fillRect(bgRect, QColor(100, 100, 100, 50));
        }
        
        // Draw tool icon
        if (toolSurfs.contains(tool)) {
            QPixmap toolPixmap = toolSurfs[tool];
            QRect toolRect(pos.x(), pos.y(), 60, 60);
            painter.drawPixmap(toolRect, toolPixmap);
        }
    }
}

void Overlay::displaySeeds(QPainter& painter)
{
    if (!player) return;
    
    // Get current seed
    QString currentSeed = player->selectedSeed;
    
    // Seed overlay positions
    QPoint seedPos(40, 140);
    int spacing = 80;
    
    QStringList seeds = {"corn", "tomato"};
    
    for (int i = 0; i < seeds.size(); ++i) {
        const QString& seed = seeds[i];
        QPoint pos = seedPos + QPoint(i * spacing, 0);
        
        // Draw seed background
        QRect bgRect(pos.x() - 5, pos.y() - 5, 70, 70);
        if (seed == currentSeed) {
            painter.fillRect(bgRect, QColor(255, 255, 255, 100));
        } else {
            painter.fillRect(bgRect, QColor(100, 100, 100, 50));
        }
        
        // Draw seed icon
        if (seedSurfs.contains(seed)) {
            QPixmap seedPixmap = seedSurfs[seed];
            QRect seedRect(pos.x(), pos.y(), 60, 60);
            painter.drawPixmap(seedRect, seedPixmap);
        }
        
        // Draw seed count
        int seedCount = player->seedInventory.value(seed, 0);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(pos.x() + 45, pos.y() + 55, QString::number(seedCount));
    }
}

void Overlay::displayInventory(QPainter& painter)
{
    if (!player) return;
    
    // Inventory display position (top right corner)
    QPoint inventoryPos(SCREEN_WIDTH - 250, 20);
    
    // Draw inventory background
    QRect inventoryBg(inventoryPos.x() - 10, inventoryPos.y() - 10, 240, 200);
    painter.fillRect(inventoryBg, QColor(0, 0, 0, 150));
    painter.setPen(QColor(255, 255, 255));
    painter.drawRect(inventoryBg);
    
    // Draw inventory title
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 14, QFont::Bold));
    painter.drawText(inventoryPos.x(), inventoryPos.y() + 15, "库存");
    
    // Draw inventory items
    painter.setFont(QFont("Arial", 12));
    int yOffset = 35;
    
    // Display regular inventory items
    for (auto it = player->inventory.begin(); it != player->inventory.end(); ++it) {
        QString itemName = it.key();
        int itemCount = it.value();
        
        QString displayText = QString("%1: %2").arg(itemName).arg(itemCount);
        painter.drawText(inventoryPos.x(), inventoryPos.y() + yOffset, displayText);
        yOffset += 20;
    }
    
    // Add separator line
    if (!player->inventory.isEmpty() && !player->seedInventory.isEmpty()) {
        painter.setPen(QColor(150, 150, 150));
        painter.drawLine(inventoryPos.x(), inventoryPos.y() + yOffset - 5, 
                        inventoryPos.x() + 200, inventoryPos.y() + yOffset - 5);
        yOffset += 10;
    }
    
    // Display seed inventory
    painter.setPen(Qt::white);
    for (auto it = player->seedInventory.begin(); it != player->seedInventory.end(); ++it) {
        QString seedName = it.key();
        int seedCount = it.value();
        
        QString displayText = QString("%1种子: %2").arg(seedName).arg(seedCount);
        painter.drawText(inventoryPos.x(), inventoryPos.y() + yOffset, displayText);
        yOffset += 20;
    }
}

void Overlay::displayMoney(QPainter& painter)
{
    if (!player) return;
    
    // Money display position (top left, below tools)
    QPoint moneyPos(40, 250);
    
    // Draw money background
    QRect moneyBg(moneyPos.x() - 10, moneyPos.y() - 10, 150, 40);
    painter.fillRect(moneyBg, QColor(255, 215, 0, 200)); // Gold background
    painter.setPen(QColor(0, 0, 0));
    painter.drawRect(moneyBg);
    
    // Draw money text
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 14, QFont::Bold));
    QString moneyText = QString("金钱: %1").arg(player->money);
    painter.drawText(moneyPos.x(), moneyPos.y() + 15, moneyText);
}

void Overlay::displayEnergy(QPainter& painter)
{
    if (!player) return;
    
    // Energy display position (top left, below money)
    QPoint energyPos(40, 300);
    
    // Draw energy background
    QRect energyBg(energyPos.x() - 10, energyPos.y() - 10, 180, 40);
    painter.fillRect(energyBg, QColor(100, 200, 100, 200)); // Green background
    painter.setPen(QColor(0, 0, 0));
    painter.drawRect(energyBg);
    
    // Draw energy bar
    int barWidth = 120;
    int barHeight = 18;
    QRect energyBarBg(energyPos.x() + 45, energyPos.y() - 4, barWidth, barHeight);
    painter.fillRect(energyBarBg, QColor(50, 50, 50)); // Dark background
    
    // Calculate energy bar fill
    float energyRatio = static_cast<float>(player->energy) / player->maxEnergy;
    int fillWidth = static_cast<int>(barWidth * energyRatio);
    QRect energyBarFill(energyPos.x() + 45, energyPos.y() - 4, fillWidth, barHeight);
    
    // Color based on energy level
    QColor energyColor;
    if (energyRatio > 0.6f) {
        energyColor = QColor(0, 255, 0); // Green
    } else if (energyRatio > 0.3f) {
        energyColor = QColor(255, 255, 0); // Yellow
    } else {
        energyColor = QColor(255, 0, 0); // Red
    }
    
    painter.fillRect(energyBarFill, energyColor);
    painter.setPen(Qt::black);
    painter.drawRect(energyBarBg);
    
    // Draw energy text
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    QString energyText = QString("体力");
    painter.drawText(energyPos.x(), energyPos.y() + 10, energyText);
    
    // Draw energy numbers
    painter.setFont(QFont("Arial", 10));
    QString energyNumbers = QString("%1/%2").arg(player->energy).arg(player->maxEnergy);
    painter.drawText(energyPos.x() + 45 + barWidth/2 - 15, energyPos.y() + 25, energyNumbers);
}

void Overlay::displayTime(QPainter& painter)
{
    if (!player || !player->level) return;
    
    // Get time from level
    int day = player->level->currentDay;
    float time = player->level->currentTime;
    int hours = static_cast<int>(time);
    int minutes = static_cast<int>((time - hours) * 60);
    
    // Create time text
    QString timeText = QString("第%1天 %2:%3")
                      .arg(day)
                      .arg(hours, 2, 10, QChar('0'))
                      .arg(minutes, 2, 10, QChar('0'));
    
    // Calculate text size for centering
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    QFontMetrics fm(painter.font());
    int textWidth = fm.horizontalAdvance(timeText);
    int textHeight = fm.height();
    
    // Position for time display (top center)
    int bgX = (SCREEN_WIDTH - textWidth) / 2 - 10;
    int bgY = 15;
    int bgWidth = textWidth + 20;
    int bgHeight = textHeight + 10;
    
    // Draw background
    painter.fillRect(bgX, bgY, bgWidth, bgHeight, QColor(0, 0, 0, 100));
    
    // Draw time text (centered in background)
    painter.setPen(Qt::white);
    QPointF timePos(bgX + 10, bgY + bgHeight - 5);
    painter.drawText(timePos, timeText);
}

void Overlay::displayWeather(QPainter& painter)
{
    if (!player || !player->level) return;
    
    // Get weather from level
    bool isRaining = player->level->isRaining;
    
    // Create weather text
    QString weatherText = isRaining ? "雨天" : "晴天";
    
    // Calculate text size for centering
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    QFontMetrics fm(painter.font());
    int textWidth = fm.horizontalAdvance(weatherText);
    int textHeight = fm.height();
    
    // Position for weather display (below time, centered)
    int bgX = (SCREEN_WIDTH - textWidth) / 2 - 10;
    int bgY = 50;
    int bgWidth = textWidth + 20;
    int bgHeight = textHeight + 10;
    
    // Draw background
    painter.fillRect(bgX, bgY, bgWidth, bgHeight, QColor(0, 0, 0, 100));
    
    // Draw weather text (centered in background)
    painter.setPen(Qt::white);
    QPointF weatherPos(bgX + 10, bgY + bgHeight - 5);
    painter.drawText(weatherPos, weatherText);
}