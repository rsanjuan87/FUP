#include "config.h"
#include <QSettings>
#include <QtWidgets/qapplication.h>

QString Config::adbDefaultPath = "adb";
QString Config::scrcpyDefaultPath = "scrcpy";
QString Config::scrcpyServerDefaultPath = "";

Config::Config(QObject *parent) : QObject(parent)
{
    load();
}

QString Config::adbPath(bool forRun) {
    return _adbPath.startsWith("!") && forRun ? adbDefaultPath : _adbPath;
}

QString Config::scrcpyPath(bool forRun) {
    return _scrcpyPath.startsWith("!") && forRun ? scrcpyDefaultPath : _scrcpyPath;
}

QString Config::scrcpyServerPath(bool forRun) {
    return _scrpcyServerPath.startsWith("!") && forRun ? scrcpyServerDefaultPath : _scrpcyServerPath;
}

void Config::setAdbPath(QString v) { _adbPath = v; }

void Config::setScrcpyPath(QString v) { _scrcpyPath = v; }

void Config::setScrcpyServerPath(QString v) { _scrpcyServerPath = v; }

void Config::load(){
    QSettings set("org.santech", "FUP");
    set.beginGroup("main");
    _adbPath = set.value("adbPath").toString();
    _scrcpyPath = set.value("scrcpyPath").toString();
    _scrpcyServerPath = set.value("scrpcyServerPath").toString();
    set.endGroup(); // main

    set.beginGroup("device");
    coherenceMode = set.value("coherence").toBool();
    set.endGroup(); // device

    emit loaded();
}

void Config::save(){
    QSettings set("org.santech", "FUP");

    set.beginGroup("main");
    set.setValue("adbPath", _adbPath);
    set.setValue("scrcpyPath", _scrcpyPath);
    set.setValue("scrpcyServerPath", _scrpcyServerPath);
    set.endGroup();//main

    set.beginGroup("device");
    set.setValue("coherence", coherenceMode);
    set.endGroup();//device


    emit saved();
}
