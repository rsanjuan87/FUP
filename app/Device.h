#ifndef DEVICE_H
#define DEVICE_H

#include <QFileInfo>
#include <QMap>
#include <QProcess>
#include <QToolButton>
#include <QDir>
#include <QDebug>
#include <QMap>
#include <QProcess>
#include <QString>
#include <QThread>
#include <QException>

#include "AppAdder.h"
#include "config.h"
#include "models/launcherinfo.h"
#include "notificationhelper.h"





class Device : public QObject {

    Q_OBJECT

public:
    //from adb devices -l
    explicit Device(QString line, Config* conf, QSystemTrayIcon *t, QObject* parent);
    ~Device();
    QString screenSize(QString screenId = "0");
    QString screenDpi(QString screenId = "0");
    QStringList screens();
    QSet<LauncherInfo*> launchers();
    QString remoteFupDir();
    QString remoteFupIconsDir();

    QString id();
    QString status();
    QString product();
    QString device();
    QString model();
    QString transportId();
    void setLine(QString line);
    bool nulled(){
        return this == nullptr || config == nullptr;
    }
    void castAudioStart(){
        QStringList params;
        params << "--no-video" << "--no-control" << "--no-window";
        runScrcpy(Defs::ActionAudio, Defs::ActionAudio, params);
    }
    void castAudioStop() {
        stopScrcpy(Defs::ActionAudio);
    }
    QString screenId(QString pkgId);
    int scrcpyCount();
public slots:

    void connectDevice();
    void disconnectDevice();
    void requestAction(QString action, std::function<void(QProcess*)> onAdbFinished = nullptr);
    void requestAction(QString action, QMap<QString, QString> extras, std::function<void (QProcess *)> onAdbFinished = nullptr);
    void getAppList();
    void init();
    // void readAdb(int);
    void readLogcat();
    void runInAdb(QString shell, std::function<void(QProcess* p)> onAdbFinished = nullptr);

    // void parse(QStringList out);
    void parseMessages(QStringList out);

    // void getIcon(QString remotePath);

    void runScrcpy(QString pkgId, QString title = "", QStringList params = QStringList(), QString screenSize="");
    void stopScrcpy(QString pkgId);
    QProcess::ProcessState scrcpyStatus(QString pkgId);

    void addedLauncherSlot(LauncherInfo *info);
    void launchersSetSlot(QSet<LauncherInfo *> list);
    void Cleared();
    void loadApps(QString path);

    void raiseWindow(QString title, int pid);
    void raiseWindow(QString pkgid, QString title);

    QString lastScreenId();

private:
    //loaded from adb
    // QString _screenSize;
    // QString _dpi;
    QString _fupDir;
    QString _lastScreenId;


    // QProcess adb;
    QProcess logcat;
    QMap<QString, QProcess*> scrcpyProcess;
    AppAdder *appAdder = nullptr;
    NotificationHelper *notifHelper;

    QString _line;
    Config* config;
    bool accessToNotif = false;
signals:
    void addLauncher(LauncherInfo*, QString );
    void launchersClearred(QString );
    void launchersSet(QSet<LauncherInfo*>, QString );

    void appClosed(QString pkgid);

    void deviceDisconected(QString);
    void deviceUpdated(QString);

protected slots:
    void deviceDisconectedSlot();

};

class DeviceDisconectedException : QException{
public:
    DeviceDisconectedException():QException(){};
};

#endif // DEVICE_H
