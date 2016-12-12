QT += core
QT -= gui

CONFIG += c++11

TARGET = est
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    estthread.cpp \
    relayracecoordinator.cpp \
    relarracer.cpp

HEADERS += \
    estthread.h \
    relayracecoordinator.h \
    relarracer.h
