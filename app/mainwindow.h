#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QMainWindow>
#include <QMouseEvent>
#include <QProcess>
#include <QPushButton>
#include <QThread>
#include <QVBoxLayout>
#include <QSystemTrayIcon>

#include "Device.h"
#include "WrapLayout.h"
#include "config.h"
#include "devicesmanager.h"
#include "server.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QSystemTrayIcon *tray, QMenu *trayMenu, HttpServer *server, QWidget *parent = nullptr);
    ~MainWindow();

    // void getIcon(QString remotePath);
    Device *currentDevice();
    void requestLoadDevices();
public slots:
    void addLauncherSlot(LauncherInfo *info, QString id);
    void launchersClearredSlot(QString);

    void loadLaunchers();
    void reloadLaunchers();
    void diconectDevice(QString id);
    void loadDevices();

    void launchersSetSlot(QSet<LauncherInfo *>, QString);

    void parceReceivedMessage(quint32, QByteArray){
        // TODO todo
    }

    void updateNotifStatus();
protected slots:
    // void loadDeviceslist(QStringList);
    void connectCurrentDevice();
    void requestAction(QString action, std::function<void (QProcess *)> onAdbFinished = nullptr);
    void requestAction(QString action, QMap<QString, QString> extras, std::function<void (QProcess *)> onAdbFinished = nullptr);
    void init();
    //void updatePackages(QStringList list);


    void onAppClick(QWidget *w);
    void on_devices_currentIndexChanged(int index);

    void addDevice(Device *dev);
    void removeDevice(QString id);
    void appClosedSlot(QString pkgid);
private slots:
    void on_devices_activated(int index);
    void on_actionCast_main_screen_toggled(bool);
    void on_actionCast_virtual_desktop_size_screen_toggled(bool v);
    void on_actionSound_toggled(bool v);

    void on_actionClear_cache_triggered();

    void on_actionSettings_triggered();

    void on_actionExit_triggered();

    void on_actionNotifications_triggered(bool checked);

private:
    Ui::MainWindow *ui;

    // QProcess adb;
    // QProcess logcat;
    // QProcess scrcpy;
    // AppAdder *appAdder;
    // NotificationHelper *notifHelper;
    // WrapLayout* appsLayout;

    Config config;
    QSystemTrayIcon *tray;
    QMenu *trayMenu;

    // int currentDeviceIndex = -1;
    QString currentDeviceId;
    WrapLayout* appsLayout;
    DevicesManager* devicesManager;
    HttpServer *server;

signals:
    void deviceDisconected(QString);
    void reloadDevices();
};




#endif // MAINWINDOW_H
