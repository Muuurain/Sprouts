#include "endinganimation.h"
#include "gamesettings.h"
#include <QPainter>
#include <QFontMetrics>
#include <QtMath>

EndingAnimation::EndingAnimation(std::function<void()> onComplete, QObject *parent)
    : QObject{parent}, onComplete(onComplete), active(false),
      animationTimer(0.0f), fadeAlpha(0.0f), totalDays(0), totalHours(0.0f),
      textRevealDuration(3.0f), totalDuration(8.0f)
{
    // Setup fonts
    titleFont.setFamily("Arial");
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    
    textFont.setFamily("Arial");
    textFont.setPointSize(18);
    
    statsFont.setFamily("Arial");
    statsFont.setPointSize(16);
    statsFont.setBold(true);
    
    // Setup colors
    backgroundColor = QColor(0, 0, 0, 255);
    textColor = QColor(255, 255, 255, 255);
    statsColor = QColor(255, 215, 0, 255); // Gold color for stats
}

void EndingAnimation::start(EndingType type, int survivedDays, float survivedHours)
{
    active = true;
    currentType = type;
    totalDays = survivedDays;
    totalHours = survivedHours;
    animationTimer = 0.0f;
    fadeAlpha = 1.0f; // Start with fade in
    
    if (type == EndingType::FAILURE) {
        setupFailureEnding();
    } else {
        setupSuccessEnding();
    }
}

void EndingAnimation::setupFailureEnding()
{
    endingText = "天有不测风云，人有旦夕祸福。\n很不幸，你在饥饿的泥泞中绝望地闭上了双眼......";
    
    // Calculate total survival time
    int totalHoursInt = (int)totalHours;
    int days = totalDays - 1; // Subtract 1 because we start from day 1
    int hours = totalHoursInt;
    
    if (days > 0) {
        statsText = QString("本次游玩成功存活：%1天%2小时").arg(days).arg(hours);
    } else {
        statsText = QString("本次游玩成功存活：%1小时").arg(hours);
    }
}

void EndingAnimation::setupSuccessEnding()
{
    endingText = "你凭着过人的智谋，在小岛上活了下来。\n同时还攒足了资源，在岛上善良的原住民\"富贵\"的帮助下，\n打造出归乡的小船。";
    
    // Calculate total survival time
    int totalHoursInt = (int)totalHours;
    int days = totalDays - 1; // Subtract 1 because we start from day 1
    int hours = totalHoursInt;
    
    if (days > 0) {
        statsText = QString("本次通关所耗费的游戏时间：%1天%2小时").arg(days).arg(hours);
    } else {
        statsText = QString("本次通关所耗费的游戏时间：%1小时").arg(hours);
    }
}

void EndingAnimation::update(float dt)
{
    if (!active) return;
    
    animationTimer += dt;
    
    // Fade in effect at start
    if (animationTimer < 1.0f) {
        fadeAlpha = 1.0f - animationTimer;
    } else {
        fadeAlpha = 0.0f;
    }
    
    // Auto finish after total duration
    if (animationTimer >= totalDuration) {
        finishAnimation();
    }
}

void EndingAnimation::handleInput(const QList<int>& pressedKeys)
{
    if (!active) return;
    
    // Only allow input after instruction is shown (textRevealDuration + 3.0f)
    if (animationTimer >= textRevealDuration + 3.0f && !pressedKeys.isEmpty()) {
        finishAnimation();
    }
}

void EndingAnimation::skip()
{
    finishAnimation();
}

void EndingAnimation::finishAnimation()
{
    active = false;
    // Exit the application
    QCoreApplication::quit();
}

void EndingAnimation::display(QPainter& painter)
{
    if (!active) return;
    
    // Fill background
    painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, backgroundColor);
    
    // Calculate text positioning
    int centerX = SCREEN_WIDTH / 2;
    int centerY = SCREEN_HEIGHT / 2;
    
    // Draw title
    painter.setFont(titleFont);
    painter.setPen(textColor);
    QString title = (currentType == EndingType::FAILURE) ? "游戏结束" : "恭喜通关！";
    QFontMetrics titleMetrics(titleFont);
    int titleWidth = titleMetrics.horizontalAdvance(title);
    painter.drawText(centerX - titleWidth / 2, centerY - 150, title);
    
    // Draw ending text with reveal animation
    painter.setFont(textFont);
    QFontMetrics textMetrics(textFont);
    
    if (animationTimer >= 1.0f) {
        QStringList lines = endingText.split("\n");
        int lineHeight = textMetrics.height() + 10;
        int totalTextHeight = lines.size() * lineHeight;
        int startY = centerY - totalTextHeight / 2;
        
        float revealProgress = qMin(1.0f, (animationTimer - 1.0f) / textRevealDuration);
        int linesToShow = (int)(lines.size() * revealProgress);
        
        for (int i = 0; i < linesToShow && i < lines.size(); ++i) {
            QString line = lines[i];
            int lineWidth = textMetrics.horizontalAdvance(line);
            int x = centerX - lineWidth / 2;
            int y = startY + i * lineHeight;
            
            // Add fade effect to current revealing line
            if (i == linesToShow - 1 && revealProgress < 1.0f) {
                float lineProgress = (lines.size() * revealProgress) - i;
                int alpha = (int)(255 * lineProgress);
                painter.setPen(QColor(textColor.red(), textColor.green(), textColor.blue(), alpha));
            } else {
                painter.setPen(textColor);
            }
            
            painter.drawText(x, y, line);
        }
    }
    
    // Draw stats text
    if (animationTimer >= textRevealDuration + 2.0f) {
        painter.setFont(statsFont);
        painter.setPen(statsColor);
        QFontMetrics statsMetrics(statsFont);
        int statsWidth = statsMetrics.horizontalAdvance(statsText);
        painter.drawText(centerX - statsWidth / 2, centerY + 100, statsText);
    }
    
    // Draw instruction (wait 1 second after text reveal completes)
    if (animationTimer >= textRevealDuration + 3.0f) {
        painter.setFont(QFont("Arial", 14));
        painter.setPen(QColor(200, 200, 200, 150));
        QString instruction = "按任意键退出游戏";
        QFontMetrics instrMetrics(painter.font());
        int instrWidth = instrMetrics.horizontalAdvance(instruction);
        painter.drawText(centerX - instrWidth / 2, SCREEN_HEIGHT - 50, instruction);
    }
    
    // Draw fade overlay
    if (fadeAlpha > 0.0f) {
        QColor fadeColor(0, 0, 0, (int)(255 * fadeAlpha));
        painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, fadeColor);
    }
}