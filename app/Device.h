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
    QString getScreenSize();

    QList<QMap<QString, QString> > convertirJsonAListaDeMapas(const QString &jsonString);

    QList<LauncherInfo> launchers;

public slots:

    void connectDevice();
    void requestAction(QString action);
    void requestAction(QString action, QMap<QString, QString> extras);
    void getAppList();
    void init();
    void readAdb(int);
    void readLogcat();
    QString runInAdb(QString shell);

    void parse(QStringList out);
    void parseMessages(QStringList out);

    void getIcon(QString remotePath);

    void runScrcpy(QStringList params = QStringList());
    void stopScrcpy();
private:
    //loaded from adb
    QString* screenSize;


    QProcess adb;
    QProcess logcat;
    QProcess scrcpy;
    AppAdder *appAdder;
    NotificationHelper *notifHelper;

    QString line;
    Config* config;
    bool accessToNotif = false;

signals:
    void addLauncher(LauncherInfo);
    void launchersClearred();
    void launchersSet(QList<LauncherInfo>);
};

#endif // DEVICE_H
