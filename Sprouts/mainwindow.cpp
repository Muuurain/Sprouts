#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "level.h"
#include "player.h"
#include <QPainter>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , level(nullptr)
    , gameTimer(nullptr)
    , elapsedTimer(nullptr)
    , lastFrameTime(0)
    , deltaTime(0.0f)
{
    ui->setupUi(this);
    setupGame();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupGame()
{
    // Set window properties
    setFixedSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    setWindowTitle("Sprout Land");
    setFocusPolicy(Qt::StrongFocus);
    
    // Initialize game components
    level = new Level(this);
    
    // Setup game timer
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::gameLoop);
    gameTimer->start(16); // ~60 FPS
    
    // Setup elapsed timer
    elapsedTimer = new QElapsedTimer();
    elapsedTimer->start();
    lastFrameTime = elapsedTimer->elapsed();
}

void MainWindow::gameLoop()
{
    // Calculate delta time
    qint64 currentTime = elapsedTimer->elapsed();
    deltaTime = (currentTime - lastFrameTime) / 1000.0f;
    lastFrameTime = currentTime;
    
    // Update input
    updateInput();
    
    // Update game
    if (level) {
        // The actual rendering will happen in paintEvent
        update(); // Trigger a repaint
    }
}

void MainWindow::updateInput()
{
    // Input is handled in keyPressEvent and keyReleaseEvent
    // Pass current pressed keys to the level/player
    if (level && level->player) {
        level->player->handleInput(pressedKeys);
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    
    static int paintCounter = 0;
    if (paintCounter % 60 == 0) { // Print every 60 frames
        qDebug() << "MainWindow: paintEvent called, frame" << paintCounter;
    }
    paintCounter++;
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    if (level) {
        level->run(deltaTime, painter, pressedKeys);
    } else {
        qDebug() << "MainWindow: No level found in paintEvent!";
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (!pressedKeys.contains(event->key())) {
        pressedKeys.append(event->key());
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    pressedKeys.removeOne(event->key());
    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Stop the game timer
    if (gameTimer) {
        gameTimer->stop();
    }
    
    event->accept();
}
