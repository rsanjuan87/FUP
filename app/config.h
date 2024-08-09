#ifndef CONFIG_H
#define CONFIG_H

#include <QMap>
#include <QObject>
#include <QSettings>




class AppConfig : public QObject
{
    Q_OBJECT
public:
    explicit AppConfig(QObject *parent = nullptr){}
    void toogleEnableCustomScreenSize(bool v){
        if(v){
            _screenSize = _screenSize.remove("!");
        }else{
            _screenSize = "!" + _screenSize;
        }
    }
    QString screenSize(bool forRun = true){
        if (forRun){
            return _screenSize.remove("!");
        }else{
            return _screenSize;
        }
    }

    void screenSize(QString v){
        _screenSize = v;
    }

    void load(QSettings *set){
        _screenSize = set->value("screenSize").toString();
    }

    void save(QSettings *set){
        set->setValue("screenSize", _screenSize);
    }

private:
    QString _screenSize;
};



class DevConfig : public QObject
{
    Q_OBJECT
public:
    explicit DevConfig(QObject *parent = nullptr) {}
    void toogleEnableCustomScreenSize(bool v) {
        if (v) {
            _screenSize = _screenSize.remove("!");
        } else {
            _screenSize = "!" + _screenSize;
        }
    }
    QString screenSize(bool forRun = true){
        if (forRun){
            return _screenSize.remove("!");
        }else{
            return _screenSize;
        }
    }

    void screenSize(QString v){
        _screenSize = v;
    }

    void load(QSettings *set){
        _screenSize = set->value("screenSize").toString();
        coherenceMode = set->value("coherenceMode").toInt();

        set->beginGroup("apps");
        QStringList akeys = set->childGroups();
        foreach(QString appId, akeys){
            set->beginGroup(appId);
            AppConfig* app = new AppConfig();
            app->load(set);
            apps.insert(appId, app);
            set->endGroup(); // appId
        }
        set->endGroup(); // apps
    }

    void save(QSettings *set){
        set->setValue("screenSize", _screenSize);
        set->setValue("coherenceMode", coherenceMode);

        set->beginGroup("apps");
        QStringList akeys = apps.keys();
        foreach(QString appId, akeys){
            set->beginGroup(appId);
            AppConfig* app = apps.value(appId);
            app->save(set);
            set->endGroup(); // appId
        }
        set->endGroup(); // apps
    }

    int coherenceMode;

    QMap<QString, AppConfig *> apps;

private:
    QString _screenSize;
};

class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(QObject *parent = nullptr);
    ~Config();
    QString adbPath(bool forRun = true);
    QString scrcpyPath(bool forRun = true);
    QString scrcpyServerPath(bool forRun = true);
    QString customScreenSize(bool forRun = true);

    static QString adbDefaultPath;
    static QString scrcpyDefaultPath;
    static QString scrcpyServerDefaultPath;

    //true -> open the apps on new virtual display
    bool coherenceMode = true;

    void setAdbPath(QString v);
    void setScrcpyPath(QString v);
    void setScrcpyServerPath(QString v);
    void customScreenSize(QString v);

    QString screenSize4Pkg(QString devId, QString pkgId);

    void setScreenSize4Pkg(QString devId, QString size, QString pkgId="");

public slots:
    void load();
    void save();
private:

#ifdef Q_OS_WIN
    QString _adbPath = "C:/platform-tools/adb.exe";
    QString _scrcpyPath = "C:/scrcpy/scrcpy.exe";
    QString _scrpcyServerPath = "C:/scrcpy/scrcpy-server";
#else
    QString _adbPath = "/Users/san/Library/Android/sdk/platform-tools/adb";
    QString _scrcpyPath = "/Users/san/Projects/FUP/app/scrcpy/builddir/app/scrcpy";
    QString _scrpcyServerPath = "/Users/san/Projects/FUP/app/scrcpy/builddir/server/scrcpy-server";
#endif

    QString _customScreenSize = "";

    QMap<QString, DevConfig*> devs;


signals:
    void loaded();
    void saved();
};
#endif // CONFIG_H
