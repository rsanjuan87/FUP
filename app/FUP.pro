QT       += core gui svg network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AppAdder.cpp \
    Defs.cpp \
    Device.cpp \
    WrapLayout.cpp \
    application.cpp \
    config.cpp \
    configdialog.cpp \
    devicesmanager.cpp \
    main.cpp \
    mainwindow.cpp \
    models/launcherinfo.cpp \
    notificationhelper.cpp \
    server.cpp

HEADERS += \
    AppAdder.h \
    Defs.h \
    Device.h \
    WrapLayout.h \
    application.h \
    config.h \
    configdialog.h \
    devicesmanager.h \
    mainwindow.h \
    models/launcherinfo.h \
    notificationhelper.h \
    server.h

FORMS += \
    configdialog.ui \
    mainwindow.ui

TRANSLATIONS += \
    FUP_en_US.ts

CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



include (QSingleApp/singleapplication.pri)
include (QAutostart/QAutostart.pri)


RESOURCES += \
    imgs.qrc
