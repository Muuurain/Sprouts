#ifndef INTROANIMATION_H
#define INTROANIMATION_H

#include <QObject>
#include <QPainter>
#include <QTimer>
#include <QString>
#include <QFont>
#include <QColor>
#include <QRect>
#include <functional>

class IntroAnimation : public QObject
{
    Q_OBJECT

public:
    explicit IntroAnimation(std::function<void()> onComplete, QObject *parent = nullptr);
    
    void start();
    void update(float dt);
    void display(QPainter& painter);
    void skip(); // Skip the animation
    bool isActive() const { return active; }
    
private:
    void setupScene1();
    void setupScene2();
    void transitionToNextScene();
    void finishAnimation();
    
    bool active;
    int currentScene;
    float sceneTimer;
    float fadeAlpha;
    bool transitioning;
    
    // Scene content
    QString currentText;
    QStringList scene1Lines;
    QStringList scene2Lines;
    int currentLineIndex;
    float textRevealTimer;
    float lineDelay;
    
    // Visual properties
    QFont titleFont;
    QFont textFont;
    QColor backgroundColor;
    QColor textColor;
    
    // Animation properties
    float sceneDuration;
    float transitionDuration;
    float textSpeed;
    
    std::function<void()> onComplete;
};

#endif // INTROANIMATION_H