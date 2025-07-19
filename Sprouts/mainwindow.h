#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QElapsedTimer>
#include <QVector>
#include <QSoundEffect>
#include "gamesettings.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Level;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void gameLoop();

private:
    void setupGame();
    void updateInput();
    
    Ui::MainWindow *ui;
    
    // Game components
    Level* level;
    QTimer* gameTimer;
    QElapsedTimer* elapsedTimer;
    
    // Input handling
    QList<int> pressedKeys;
    
    // Game timing
    qint64 lastFrameTime;
    float deltaTime;
};

#endif // MAINWINDOW_H
