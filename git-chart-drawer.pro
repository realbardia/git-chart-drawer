QT += widgets charts
CONFIG += c++17
VERSION = 0.1.0

include(configurations/configurations.pri)
include(fonts/fonts.pri)

SOURCES += \
    abstractchartwidget.cpp \
    commitchartwidget.cpp \
    gitcommands.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    abstractchartwidget.h \
    commitchartwidget.h \
    gitcommands.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    resource.qrc
