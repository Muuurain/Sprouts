#ifndef ENDINGANIMATION_H
#define ENDINGANIMATION_H

#include <QObject>
#include <QPainter>
#include <QString>
#include <QFont>
#include <QColor>
#include <QCoreApplication>
#include <functional>

enum class EndingType {
    FAILURE,
    SUCCESS
};

class EndingAnimation : public QObject
{
    Q_OBJECT

public:
    explicit EndingAnimation(std::function<void()> onComplete, QObject *parent = nullptr);
    
    void start(EndingType type, int survivedDays, float survivedHours);
    void update(float dt);
    void display(QPainter& painter);
    void handleInput(const QList<int>& pressedKeys);
    void skip();
    bool isActive() const { return active; }
    
private:
    void setupFailureEnding();
    void setupSuccessEnding();
    void finishAnimation();
    
    bool active;
    EndingType currentType;
    float animationTimer;
    float fadeAlpha;
    
    // Game stats
    int totalDays;
    float totalHours;
    
    // Text content
    QString endingText;
    QString statsText;
    
    // Visual properties
    QFont titleFont;
    QFont textFont;
    QFont statsFont;
    QColor backgroundColor;
    QColor textColor;
    QColor statsColor;
    
    // Animation properties
    float textRevealDuration;
    float totalDuration;
    
    std::function<void()> onComplete;
};

#endif // ENDINGANIMATION_H