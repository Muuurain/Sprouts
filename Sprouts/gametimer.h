#ifndef GAMETIMER_H
#define GAMETIMER_H

#include <QObject>
#include <QTimer>
#include <functional>

class GameTimer : public QObject
{
    Q_OBJECT

public:
    explicit GameTimer(int duration, std::function<void()> callback = nullptr, QObject *parent = nullptr);
    
    // Timer control
    void activate();
    void deactivate();
    void update();
    
    // Properties
    bool isActive() const { return active; }
    int getDuration() const { return duration; }
    void setDuration(int newDuration) { duration = newDuration; }
    
private slots:
    void onTimeout();
    
private:
    QTimer* timer;
    int duration; // in milliseconds
    bool active;
    std::function<void()> callback;
};

#endif // GAMETIMER_H