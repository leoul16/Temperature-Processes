QT += core
QT -= gui

TARGET = Yiheyis_Leoul_hw5
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11
LIBS   += -lrt

TEMPLATE = app

SOURCES += external.cpp \
    main.cpp

DISTFILES += \
    launch.sh

