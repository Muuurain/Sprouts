QT       += core gui widgets multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    resourceloader.cpp \
    gametimer.cpp \
    sprite.cpp \
    spritegroup.cpp \
    tree.cpp \
    player.cpp \
    level.cpp \
    soillayer.cpp \
    plant.cpp \
    overlay.cpp \
    sky.cpp \
    transition.cpp \
    menu.cpp \
    introanimation.cpp \
    endinganimation.cpp

HEADERS += \
    mainwindow.h \
    gamesettings.h \
    resourceloader.h \
    gametimer.h \
    sprite.h \
    spritegroup.h \
    tree.h \
    player.h \
    soillayer.h \
    plant.h \
    overlay.h \
    sky.h \
    transition.h \
    menu.h \
    level.h \
    introanimation.h \
    endinganimation.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
