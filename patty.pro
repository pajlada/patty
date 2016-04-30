#-------------------------------------------------
#
# Project created by QtCreator 2016-04-29T00:20:38
#
#-------------------------------------------------

QT       += core gui
QT       += network

CONFIG += communi
COMMUNI += core model util

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = patty
TEMPLATE = app

include(lib/libcommuni/src/src.pri)


SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/ircclient.cpp

HEADERS  += src/mainwindow.h \
    src/ircclient.h

FORMS    += src/ui/mainwindow.ui
