#-------------------------------------------------
#
# Project created by QtCreator 2013-05-13T08:05:23
#
#-------------------------------------------------

QT           += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET        = wasta-backup

TEMPLATE      = app

SOURCES      += main.cpp \
    mainwindow.cpp

TRANSLATIONS += \
    l10n/wasta-backup_es.ts \
    l10n/wasta-backup_fr.ts \
    l10n/wasta-backup_ru.ts

HEADERS      += mainwindow.h \
    ui_mainwindow.h

FORMS        += mainwindow.ui

CONFIG       += qtestlib

OTHER_FILES  += \
    readme.txt \
    wasta-backup.appdata.xml \
    wasta-backup.desktop \
    wasta-backup.rules \
    wasta-backup-udev \
    wasta-backup-usb-autolaunch

DISTFILES    += \
    l10n/wasta-backup_es.ts \
    l10n/wasta-backup_fr.ts \
    l10n/wasta-backup_ru.ts
