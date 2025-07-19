#include "transition.h"
#include "gamesettings.h"
#include <QPainter>
#include <functional>

Transition::Transition(std::function<void()> resetFunc, Player* player, QObject *parent)
    : QObject{parent}, reset(resetFunc), player(player)
{
    color = QColor(255, 255, 255);
    speed = -2; // Start with fade in
    active = false;
}

void Transition::play()
{
    active = true;
    speed = 2; // Fade out first
}

void Transition::update(float dt)
{
    if (!active) return;
    
    // Update alpha
    int alpha = color.alpha();
    alpha += static_cast<int>(speed * 255 * dt); // Convert speed to alpha change per second
    alpha = qBound(0, alpha, 255);
    color.setAlpha(alpha);
    
    // Check for transition phases
    if (speed > 0 && alpha >= 255) {
        // Fully faded out, call reset and start fading in
        if (reset) {
            reset();
        }
        speed = -2; // Start fading in
    } else if (speed < 0 && alpha <= 0) {
        // Fully faded in, transition complete
        active = false;
        speed = 2; // Reset for next transition
    }
}

void Transition::display(QPainter& painter)
{
    if (!active) return;
    
    // Draw transition overlay
    painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
}