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

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE





class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QSystemTrayIcon *tray, QMenu *trayMenu, QWidget *parent = nullptr);
    ~MainWindow();

    void getIcon(QString remotePath);
public slots:
    void loadDeviceslist();
protected slots:
    void connectDevice();
    void requestAction(QString action);
    void requestAction(QString action, QMap<QString, QString> extras);
    void init();
    //void updatePackages(QStringList list);

    void on_toolButton_clicked();

    void onAppClick(QWidget *w);
    void on_devices_currentIndexChanged(int index);

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
    QList<Device*> devices;
    int currentDeviceIndex = -1;
    WrapLayout* appsLayout;
};



#endif // MAINWINDOW_H
