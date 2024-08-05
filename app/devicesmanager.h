#ifndef DEVICESMANAGER_H
#define DEVICESMANAGER_H

#include "Device.h"
#include "config.h"

#include <QObject>
#include <QProcess>
#include <QThread>

class DevicesManager : public QObject //Thread
{
    Q_OBJECT
public:
    explicit DevicesManager(Config *config, QSystemTrayIcon *tray, QObject* parent);
    ~DevicesManager();

    bool isEmpty();
    Device* value(QString id);
    void deviceRemovedSlot(QString id);
public slots:
    void load(){
        start();
        // run();
    };


    void diconectDevice(QString id);

protected:
    QSystemTrayIcon* tray;
    Config *config;
    QMap<QString, Device*> devices;
    // void run();// override;

protected slots:

    // void addLauncherSlot(LauncherInfo*, QString );
    // void launchersClearredSlot(QString );
    // void launchersSetSlot(QSet<LauncherInfo*>, QString );

    void deviceDisconectedSlot(QString);

    void start();
    void readOut(int);
signals:
    void deviceAdded(Device*);
    void deviceUpdated(Device*);
    void deviceRemoved(QString);
    void devicesConnected(QList<Device*>);


    void addLauncher(LauncherInfo*, QString );
    void launchersClearred(QString );
    void launchersSet(QSet<LauncherInfo*>, QString );

    void deviceDisconected(QString);
private:
    QProcess *p;
};

#endif // DEVICESMANAGER_H
