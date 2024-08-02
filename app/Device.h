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

#include "AppAdder.h"
#include "config.h"
#include "models/launcherinfo.h"
#include "notificationhelper.h"





class Device : public QObject {

    Q_OBJECT

public:
    //from adb devices -l
    explicit Device(QString line, Config* conf, QSystemTrayIcon *t, QObject* parent = 0);
    ~Device();
    QString id();
    QString screenSize();
    QString screenDpi();
    QStringList screens();
    QSet<LauncherInfo*> launchers();
    QString remoteFupDir();
    QString remoteFupIconsDir();

public slots:

    void connectDevice();
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

    void runScrcpy(QString pkgId,QString title, QStringList params = QStringList());
    void stopScrcpy(QString pkgId);
    QProcess::ProcessState scrcpyStatus(QString pkgId);

    void addedLauncherSlot(LauncherInfo *info);
    void launchersSetSlot(QSet<LauncherInfo *> list);
    void launchersClearredSlot();
    void loadApps(QString path);

private:
    //loaded from adb
    QString _screenSize;
    QString _dpi;
    QString _fupDir;


    // QProcess adb;
    QProcess logcat;
    QMap<QString, QProcess*> scrcpy;
    AppAdder *appAdder;
    NotificationHelper *notifHelper;

    QString line;
    Config* config;
    bool accessToNotif = false;

signals:
    void addLauncher(LauncherInfo*, QString );
    void launchersClearred(QString );
    void launchersSet(QSet<LauncherInfo*>, QString );

private:
};

#endif // DEVICE_H
