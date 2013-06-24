#-------------------------------------------------
#
# Project created by QtCreator 2013-05-13T08:05:23
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = wasta-backup
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

CONFIG += qtestlib

OTHER_FILES += \
    README.txt \
    wasta-base.png \
    wasta-backup-usb-autolaunch.sh \
    wasta-backup-udev.sh \
    wasta-backup.xcf \
    wasta-backup.rules \
    wasta-backup.png \
    wasta-backup.desktop \
    control
