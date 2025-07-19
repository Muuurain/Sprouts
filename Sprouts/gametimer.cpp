#include "gametimer.h"

GameTimer::GameTimer(int duration, std::function<void()> callback, QObject *parent)
    : QObject{parent}, duration(duration), active(false), callback(callback)
{
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &GameTimer::onTimeout);
}

void GameTimer::activate()
{
    if (!active) {
        active = true;
        timer->start(duration);
    }
}

void GameTimer::deactivate()
{
    if (active) {
        active = false;
        timer->stop();
    }
}

void GameTimer::update()
{
    // This method can be used for manual timer updates if needed
    // Currently, Qt's QTimer handles the timing automatically
}

void GameTimer::onTimeout()
{
    active = false;
    if (callback) {
        callback();
    }
}