#include "config.h"
#include <QDir>
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
    if(_adbPath.startsWith("!") && forRun){
        return adbDefaultPath ;
    }
    return _adbPath;
}

QString Config::scrcpyPath(bool forRun) {
    if(_scrcpyPath.startsWith("!") && forRun) {
        return   scrcpyDefaultPath ;
    }
    return _scrcpyPath;
}

QString Config::scrcpyServerPath(bool forRun) {
    if(_scrpcyServerPath.startsWith("!") && forRun){
        return scrcpyServerDefaultPath ;
    }
    return _scrpcyServerPath;
}

QString Config::customScreenSize(bool forRun){
    if(forRun){
        return _customScreenSize.startsWith("!")
        ? ""
        : _customScreenSize.remove("!");
    }else{
        return _customScreenSize;
    }
}

void Config::setAdbPath(QString v) { _adbPath = v; }

void Config::setScrcpyPath(QString v) { _scrcpyPath = v; }

void Config::setScrcpyServerPath(QString v) { _scrpcyServerPath = v; }

void Config::load(){
    QSettings set(QDir::homePath()+"/.config/org.santech.FUP",QSettings::IniFormat);
    set.beginGroup("main");
    _adbPath = set.value("adbPath", _adbPath /*"!"+adbDefaultPath*/).toString();
    _scrcpyPath = set.value("scrcpyPath", _scrcpyPath/*"!"+scrcpyDefaultPath*/).toString();
    _scrpcyServerPath = set.value("scrpcyServerPath", _scrpcyServerPath/*"!"+scrcpyServerDefaultPath*/).toString();
    set.endGroup(); // main

    set.beginGroup("device");
    coherenceMode = set.value("coherence", true).toBool();
    _customScreenSize = set.value("customScreenSize").toString();
    set.endGroup(); // device

    set.beginGroup("devices");
    QStringList dkeys = set.childGroups();
    foreach(QString devId, dkeys){
        set.beginGroup(devId);
        DevConfig* dev = new DevConfig();
        dev->load(&set);
        devs.insert(devId, dev);
        set.endGroup(); // devId
    }
    set.endGroup(); // devices
    emit loaded();
}

void Config::save(){
    QSettings set(QDir::homePath()+"/.config/org.santech.FUP",QSettings::IniFormat);

    set.beginGroup("main");
    set.setValue("adbPath", _adbPath);
    set.setValue("scrcpyPath", _scrcpyPath);
    set.setValue("scrpcyServerPath", _scrpcyServerPath);
    set.endGroup();//main

    set.beginGroup("device");
    set.setValue("coherence", coherenceMode);
    set.setValue("customScreenSize", _customScreenSize);
    set.endGroup();//device



    set.beginGroup("devices");
    QStringList dkeys = devs.keys();
    foreach(QString devId, dkeys){
        set.beginGroup(devId);
        DevConfig* dev = devs.value(devId);
        dev->save(&set);
        set.endGroup(); // devId
    }
    set.endGroup(); // devices


    emit saved();
}

void Config::customScreenSize(QString v) { _customScreenSize = v; }

Config::~Config() {
    while (devs.size() > 0) {
        DevConfig *dev = devs.take(devs.firstKey());
        while (dev->apps.size() > 0) {
            AppConfig *app = dev->apps.take(dev->apps.firstKey());
            delete app;
        }
        delete dev;
    }
}

QString Config::screenSize4Pkg(QString devId, QString pkgId) {
    if (devs.contains(devId)){
        DevConfig *dev = devs.value(devId);
        if(dev->apps.contains(pkgId)){
            return dev->apps.value(pkgId)->screenSize();
        }else{
            return dev->screenSize();
        }
    }else{
        return "";
    }
}

void Config::setScreenSize4Pkg(QString devId, QString size, QString pkgId) {
    DevConfig* dev;
    if(devs.contains(devId)){
        dev = devs.value(devId);
    }else{
        dev = new DevConfig();
    }
    if(pkgId.isEmpty()){
        dev->screenSize(size);
    }else{
        AppConfig* app;
        if(dev->apps.contains(pkgId)){
            app = dev->apps.value(pkgId);
        }else{
            app = new AppConfig();
        }
        app->screenSize(size);
        dev->apps.insert(pkgId, app);
    }
    devs.insert(devId, dev);
    save();
}
