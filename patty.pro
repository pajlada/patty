#-------------------------------------------------
#
# Project created by QtCreator 2016-04-29T00:20:38
#
#-------------------------------------------------

QT       += core gui
QT       += network

CONFIG += communi c++11
COMMUNI += core model util

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = patty
TEMPLATE = app

include(lib/libcommuni/src/src.pri)


SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/ircclient.cpp \
    src/emotemanager.cpp \
    src/loginwindow.cpp \
    src/animatedtextbrowser.cpp

HEADERS  += src/mainwindow.h \
    src/ircclient.h \
    src/emotemanager.h \
    src/loginwindow.h \
    src/animatedtextbrowser.h

RESOURCES += lib/QDarkStyleSheet/qdarkstyle/style.qrc

FORMS    += src/ui/mainwindow.ui \
    src/ui/loginwindow.ui
