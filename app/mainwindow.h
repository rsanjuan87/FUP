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

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QSystemTrayIcon *tray, QMenu *trayMenu, QWidget *parent = nullptr);
    ~MainWindow();

    // void getIcon(QString remotePath);
    Device *currentDevice();
public slots:
    void addLauncherSlot(LauncherInfo *info, QString id);
    void launchersClearredSlot(QString);

    void loadLaunchers();
    void reloadLaunchers();
    void diconectDevice(QString id);
    void loadDevices();

    void launchersSetSlot(QSet<LauncherInfo *>, QString);
protected slots:
    // void loadDeviceslist(QStringList);
    void connectCurrentDevice();
    void requestAction(QString action, std::function<void (QProcess *)> onAdbFinished = nullptr);
    void requestAction(QString action, QMap<QString, QString> extras, std::function<void (QProcess *)> onAdbFinished = nullptr);
    void init();
    //void updatePackages(QStringList list);

    void on_toolButton_clicked();

    void onAppClick(QWidget *w);
    void on_devices_currentIndexChanged(int index);

    void addDevice(Device *dev);
    void removeDevice(QString id);
private slots:
    void on_devices_activated(int index);

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

signals:
    void deviceDisconected(QString);
};




#endif // MAINWINDOW_H
