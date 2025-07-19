#ifndef OVERLAY_H
#define OVERLAY_H

#include <QObject>
#include <QPainter>
#include <QPixmap>
#include <QMap>
#include "gamesettings.h"

class Player;

class Overlay : public QObject
{
    Q_OBJECT

public:
    explicit Overlay(Player* player, QObject *parent = nullptr);
    
    // Display overlay
    void display(QPainter& painter);
    
private:
    void displayTools(QPainter& painter);
    void displaySeeds(QPainter& painter);
    void displayInventory(QPainter& painter);
    void displayMoney(QPainter& painter);
    void displayEnergy(QPainter& painter);
    void displayTime(QPainter& painter);
    void displayWeather(QPainter& painter);
    Player* player;
    
    // Tool and seed surfaces
    QMap<QString, QPixmap> toolSurfs;
    QMap<QString, QPixmap> seedSurfs;
    
    // Load graphics
    void loadGraphics();
};

#endif // OVERLAY_H