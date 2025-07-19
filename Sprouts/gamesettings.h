#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include <QPoint>
#include <QPointF>
#include <QMap>
#include <QVector>
#include <QString>

// Screen constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int TILE_SIZE = 64;

// Layer enumeration for rendering order
enum Layer {
    WATER = 0,
    GROUND = 1,
    SOIL = 2,
    SOIL_WATER = 3,
    RAIN_FLOOR = 4,
    HOUSE_BOTTOM = 5,
    GROUND_PLANT = 6,
    MAIN = 7,
    HOUSE_TOP = 8,
    FRUIT = 9,
    RAIN = 10,
    RAIN_DROPS = 11
};

// Tool types
enum ToolType {
    HOE,
    AXE,
    WATER_TOOL
};

// Seed types
enum SeedType {
    CORN,
    TOMATO
};

// Player status
enum PlayerStatus {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    UP_IDLE,
    DOWN_IDLE,
    LEFT_IDLE,
    RIGHT_IDLE,
    UP_HOE,
    DOWN_HOE,
    LEFT_HOE,
    RIGHT_HOE,
    UP_AXE,
    DOWN_AXE,
    LEFT_AXE,
    RIGHT_AXE,
    UP_WATER,
    DOWN_WATER,
    LEFT_WATER,
    RIGHT_WATER
};

// Overlay positions
const QPoint TOOL_OVERLAY_POS(40, SCREEN_HEIGHT - 15);
const QPoint SEED_OVERLAY_POS(70, SCREEN_HEIGHT - 5);

// Player tool offsets
const QMap<QString, QPointF> PLAYER_TOOL_OFFSET = {
    {"left", QPointF(-50, 40)},
    {"right", QPointF(50, 40)},
    {"up", QPointF(0, -10)},
    {"down", QPointF(0, 50)}
};

// Apple positions for different tree sizes
const QMap<QString, QVector<QPoint>> APPLE_POS = {
    {"Small", {{18,17}, {30,37}, {12,50}, {30,45}, {20,30}, {30,10}}},
    {"Large", {{30,24}, {60,65}, {50,50}, {16,40}, {45,50}, {42,70}}}
};

// Growth speeds
const QMap<QString, float> GROW_SPEED = {
    {"corn", 1.0f},
    {"tomato", 0.7f}
};

// Growth speed constant
const int GROW_SPEED_VALUE = 3;

// Sale prices
const QMap<QString, int> SALE_PRICES = {
    {"wood", 4},
    {"apple", 2},
    {"corn", 10},
    {"tomato", 20}
};

// Purchase prices
const QMap<QString, int> PURCHASE_PRICES = {
    {"corn", 4},
    {"tomato", 5}
};

#endif // GAMESETTINGS_H