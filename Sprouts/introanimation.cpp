#include "introanimation.h"
#include "gamesettings.h"
#include <QPainter>
#include <QFontMetrics>
#include <QtMath>

IntroAnimation::IntroAnimation(std::function<void()> onComplete, QObject *parent)
    : QObject{parent}, onComplete(onComplete), active(false), currentScene(0),
      sceneTimer(0.0f), fadeAlpha(0.0f), transitioning(false),
      currentLineIndex(0), textRevealTimer(0.0f), lineDelay(2.0f),
      sceneDuration(15.0f), transitionDuration(2.0f), textSpeed(30.0f)
{
    // Setup fonts
    titleFont.setFamily("Arial");
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    
    textFont.setFamily("Arial");
    textFont.setPointSize(16);
    
    // Setup colors
    backgroundColor = QColor(0, 0, 0, 255);
    textColor = QColor(255, 255, 255, 255);
    
    setupScene1();
    setupScene2();
}

void IntroAnimation::setupScene1()
{
    scene1Lines.clear();
    scene1Lines << "那是一个月黑风高的早上，英姿飒爽，气宇轩昂，训练有素的你";
    scene1Lines << "（也就是我们的主角bower）驾驶着\"山东船\"执行机密任务，";
    scene1Lines << "你乘风破浪，劈波斩浪，踏浪而行。";
    scene1Lines << "";
    scene1Lines << "可惜......自然的力量实在是太过磅礴，";
    scene1Lines << "一阵波涛汹涌，汹涌澎湃，惊涛骇浪袭来！";
    scene1Lines << "船还是被冰冷的深海吞没！";
    scene1Lines << "";
    scene1Lines << "海水如同脱缰的野马般横冲直撞进你的嘴里，";
    scene1Lines << "强烈的窒息感紧紧地扼住你咽喉，";
    scene1Lines << "你绝望却又坚定地抓住甲板上一根铁链......";
    scene1Lines << "";
    scene1Lines << "(意识逐渐模糊）";
}

void IntroAnimation::setupScene2()
{
    scene2Lines.clear();
    scene2Lines << "强烈的眩晕感，刺骨的海水如同噩梦般缠绕着你。";
    scene2Lines << "求生欲将你唤醒。";
    scene2Lines << "你在一座未知的海岛上缓缓醒来。";
    scene2Lines << "";
    scene2Lines << "但是上天永远眷顾着你，我亲爱的bower。";
    scene2Lines << "这是一座长满了tomato and corn的小岛";
    scene2Lines << "同时还有一些未知的机遇（这是你逃生的关键），";
    scene2Lines << "";
    scene2Lines << "机智如你，一定能合理利用小岛上有限的生存资源";
    scene2Lines << "完成活下去的基本目标，也许也还能......";
    scene2Lines << "";
    scene2Lines << "亲爱的勇士，准备好了吗，";
    scene2Lines << "开启你的海岛求生之旅吧！";
}

void IntroAnimation::start()
{
    active = true;
    currentScene = 1;
    sceneTimer = 0.0f;
    fadeAlpha = 0.0f;
    transitioning = false;
    currentLineIndex = 0;
    textRevealTimer = 0.0f;
}

void IntroAnimation::update(float dt)
{
    if (!active) return;
    
    sceneTimer += dt;
    
    if (transitioning) {
        // Handle transition fade effect
        fadeAlpha += dt / transitionDuration;
        if (fadeAlpha >= 1.0f) {
            fadeAlpha = 1.0f;
            transitioning = false;
            
            if (currentScene == 1) {
                currentScene = 2;
                currentLineIndex = 0;
                textRevealTimer = 0.0f;
                sceneTimer = 0.0f;
            } else {
                finishAnimation();
                return;
            }
        }
    } else {
        // Handle text reveal
        textRevealTimer += dt;
        
        QStringList* currentLines = (currentScene == 1) ? &scene1Lines : &scene2Lines;
        
        if (textRevealTimer >= lineDelay && currentLineIndex < currentLines->size()) {
            currentLineIndex++;
            textRevealTimer = 0.0f;
        }
        
        // Check if scene should transition
        if (sceneTimer >= sceneDuration || currentLineIndex >= currentLines->size()) {
            transitionToNextScene();
        }
        
        // Fade in effect at scene start
        if (sceneTimer < 1.0f) {
            fadeAlpha = 1.0f - sceneTimer;
        } else {
            fadeAlpha = 0.0f;
        }
    }
}

void IntroAnimation::transitionToNextScene()
{
    transitioning = true;
    fadeAlpha = 0.0f;
}

void IntroAnimation::skip()
{
    finishAnimation();
}

void IntroAnimation::finishAnimation()
{
    active = false;
    if (onComplete) {
        onComplete();
    }
}

void IntroAnimation::display(QPainter& painter)
{
    if (!active) return;
    
    // Fill background
    painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, backgroundColor);
    
    // Get current lines to display
    QStringList* currentLines = (currentScene == 1) ? &scene1Lines : &scene2Lines;
    
    // Calculate text area
    int margin = 100;
    int textAreaWidth = SCREEN_WIDTH - 2 * margin;
    int textAreaHeight = SCREEN_HEIGHT - 2 * margin;
    int startY = margin + 100;
    
    // Draw scene title
    painter.setFont(titleFont);
    painter.setPen(textColor);
    QString title = (currentScene == 1) ? "第一章：海难" : "第二章：重生";
    QFontMetrics titleMetrics(titleFont);
    int titleWidth = titleMetrics.horizontalAdvance(title);
    painter.drawText((SCREEN_WIDTH - titleWidth) / 2, startY - 50, title);
    
    // Draw text lines
    painter.setFont(textFont);
    QFontMetrics textMetrics(textFont);
    int lineHeight = textMetrics.height() + 10;
    
    for (int i = 0; i < qMin(currentLineIndex, currentLines->size()); ++i) {
        QString line = currentLines->at(i);
        if (!line.isEmpty()) {
            // Center the text
            int lineWidth = textMetrics.horizontalAdvance(line);
            int x = (SCREEN_WIDTH - lineWidth) / 2;
            int y = startY + i * lineHeight;
            
            // Add subtle animation to current line
            if (i == currentLineIndex - 1 && textRevealTimer < 0.5f) {
                int alpha = (int)(255 * (textRevealTimer / 0.5f));
                painter.setPen(QColor(textColor.red(), textColor.green(), textColor.blue(), alpha));
            } else {
                painter.setPen(textColor);
            }
            
            painter.drawText(x, y, line);
        }
    }
    
    // Draw fade overlay for transitions
    if (fadeAlpha > 0.0f) {
        QColor fadeColor(0, 0, 0, (int)(255 * fadeAlpha));
        painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, fadeColor);
    }
    
    // Draw skip instruction
    painter.setFont(QFont("Arial", 12));
    painter.setPen(QColor(200, 200, 200, 150));
    painter.drawText(SCREEN_WIDTH - 200, SCREEN_HEIGHT - 30, "按任意键跳过");
}