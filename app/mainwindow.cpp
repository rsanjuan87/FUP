#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Defs.h"

#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QStringList>
#include <QToolButton>
#include <QMenu>
#include <QMessageBox>
#include <QApplication>
#include <QScreen>


MainWindow::MainWindow(QSystemTrayIcon *t, QMenu *trayMenu, QWidget *parent)    :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    tray(t),
    trayMenu(trayMenu),
    appsLayout(new WrapLayout(0))

{
    ui->setupUi(this);

    init();

    //getAppList();
    requestAction(Defs::KEY_GET_LAUNCHERS);

    ui->appListLayout->insertLayout(0, appsLayout);
}

MainWindow::~MainWindow()
{
    delete ui;
    int count = devices.size();
    for (int i = 0; i < count ; i++) {
        Device* dev = devices.at(0);
        devices.removeFirst();
        delete dev;
    }
    tray->hide();
}

void MainWindow::init(){
    deviceListLoader = new DeviceListLoader(&config);
    connect(deviceListLoader, SIGNAL(devices(QStringList)), this, SLOT(loadDeviceslist(QStringList)));

    trayMenu->addAction("Reconectar", this, &MainWindow::connectDevice);
    trayMenu->addSeparator();
    trayMenu->addAction("Mostrar Lanzador", this, SLOT(show()));
    trayMenu->addSeparator();
    trayMenu->addAction("Salir", qApp, SLOT(quit()));

    trayMenu->setDefaultAction(trayMenu->actions().first());

    deviceListLoader->start();
}

void MainWindow::loadDeviceslist(QStringList lines){
    devices.clear();

    foreach (QString l, lines) {
        if(l.contains("device ") || l.contains("offline ")){
            Device* dev = new Device(l, &config, tray);
            devices.append(dev);
            QString status = "";
            if(l.contains("offline")){
                status = ":off";
            }
            ui->devices->addItem(dev->id()+status);
        }
    }
    if (currentDeviceIndex == -1 && !devices.isEmpty() ){
        currentDeviceIndex = 0;
        on_devices_currentIndexChanged(currentDeviceIndex);
    }

    connectDevice();
}

void MainWindow::connectDevice()
{
    if(currentDeviceIndex != -1 && !devices.isEmpty() ){
        devices[currentDeviceIndex]->connectDevice();
        // reloadLaunchers();
    }
}


void MainWindow::requestAction(QString action, std::function<void(QProcess*)> onAdbFinished){
    if(currentDeviceIndex != -1 && !devices.isEmpty() ){
        return devices[currentDeviceIndex]->requestAction(action, onAdbFinished);
    }
}

void MainWindow::requestAction(QString action, QMap<QString, QString> extras, std::function<void(QProcess*)> onAdbFinished) {
    if(currentDeviceIndex != -1 && !devices.isEmpty() ){
        return devices[currentDeviceIndex]->requestAction(action, extras, onAdbFinished);
    }
}




//enable access notification
//9 and upper
// adb shell cmd notification allow_listener org.santech.notifyme/org.santech.notifyme.NotificationService
// adb shell cmd notification allow_listener/disallow_listener <package_name/notification_listener_class_name>
// adb shell settings put secure enabled_notification_listeners  <package_name/notification_listener_class_name>

//get current notifications from desktop app
// adb shell dumpsys notification --noredact | sed -n /extras/,/stats/p
// outpput cercano a un json
/** extras={
          android.title=String (Seguridad se está ejecutando)
          android.reduced.images=Boolean (true)
          android.text=String (Presiona para obtener más información o detener la app.)
          android.appInfo=ApplicationInfo (ApplicationInfo{72d36b8 com.miui.securitycenter})
      }
      stats=SingleNotificationStats{posttimeElapsedMs=68120, posttimeToFirstClickMs=-1, posttimeToDismissMs=-1, airtimeCount=0, airtimeMs=0, currentAirtimeStartElapsedMs=-1, airtimeExpandedMs=0, posttimeToFirstVisibleExpansionMs=-1, currentAirtimeExpandedStartElapsedMs=-1, requestedImportance=3, naturalImportance=5, isNoisy=true}
**/


//start broadcast
//adb shell 'am broadcast -a android.intent.action.VIEW -n org.santech.notifyme/org.santech.notifyme.BroadcastReceiver --es sms_body "test from adb"'


void MainWindow::on_toolButton_clicked()
{
    //requestAction(Defs::KEY_GET_LAUNCHERS);
    reloadLaunchers();
}


void MainWindow::clearLaunchers(QString id){
    if (currentDevice()->id() != id){
        return;
    }
    launchers.clear();
    while (QLayoutItem *item = appsLayout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->hide();
            widget->deleteLater();
        }
        delete item;
    }
    // ui->appListLayout->layout()->removeItem(appsLayout);
    // delete appsLayout;
}

void MainWindow::on_devices_currentIndexChanged(int index)
{
    if (currentDeviceIndex != -1){
        disconnect(currentDevice(), SIGNAL(addLauncher(LauncherInfo*,QString)));
        disconnect(currentDevice(), SIGNAL(launchersClearred(QString)));
    }
    currentDeviceIndex = index;
    connectDevice();
    reloadLaunchers();
}


void MainWindow::reloadLaunchers(){
    clearLaunchers(currentDevice()->id());
    loadLaunchers();
}

void MainWindow::loadLaunchers(){

    // appsLayout = new WrapLayout(0);
    // ui->appListLayout->insertLayout(0, appsLayout);
    connect(currentDevice(), SIGNAL(addLauncher(LauncherInfo*,QString)), this, SLOT(addLauncher(LauncherInfo*, QString)));
    connect(currentDevice(), SIGNAL(launchersClearred(QString)), this, SLOT(clearLaunchers(QString)));
    // Device* device = currentDevice();
    // int count = device->launchers().size();
    // for (int j = 0; j < count; j++) {
    //     LauncherInfo* info = device->launchers().at(j);
    //     addLauncher(info, device->id());
    // }
}

void MainWindow::addLauncher(LauncherInfo* info, QString id){
    QIcon icon(Defs::localIconsPath(id)+info->pkgId);
    if(id != currentDevice()->id()){
        return;
    }
    if(launchers.contains(info))return;
    launchers.insert(info);
    QToolButton *btn = new QToolButton(0);
    btn->setIcon(icon);
    btn->setIconSize(QSize(56, 56));
    btn->setText(info->label);
    btn->setProperty(Defs::KEY_GET_PACKAGES.toUtf8(), info->pkgId);
    btn->setProperty(Defs::KEY_GET_LAUNCHERS.toUtf8(), info->activityId);
    btn->setProperty(Defs::KEY_GET_LABEL.toUtf8(), info->label);
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn->setAutoRaise(false);
    btn->setFixedWidth(70);
    btn->setToolTip(info->label);
    connect(btn, &QToolButton::clicked, this, [this, btn]() { onAppClick(btn); });
    btn->setFocusPolicy(Qt::TabFocus);
    appsLayout->addWidget(btn);
}


void MainWindow::onAppClick(QWidget *w){

    if(currentDeviceIndex == -1 || devices.isEmpty() ){
        return;
    }


    QString pkgId = w->property(Defs::KEY_GET_PACKAGES.toUtf8()).toString();
    QString actyId = w->property(Defs::KEY_GET_LAUNCHERS.toUtf8()).toString();
    QString actyLabel = w->property(Defs::KEY_GET_LABEL.toUtf8()).toString();


    QString id = config.coherenceMode ? pkgId : "_";
    QProcess::ProcessState status = devices[currentDeviceIndex]->scrcpyStatus(id);
    QString screenId = "0";
    if(status != QProcess::Running){
        devices[currentDeviceIndex]->stopScrcpy(id);
        QStringList screens = devices[currentDeviceIndex]->screens();
        devices[currentDeviceIndex]->runScrcpy(id, actyLabel);
        QStringList screens1 = devices[currentDeviceIndex]->screens();
        if (screens.length() == 0){
            QMessageBox::critical(this, "Error", "Error getting displays on "+currentDevice()->id());
            return;
        }
        while(screens.length() >= screens1.length() ){
            screens1 = devices[currentDeviceIndex]->screens();
        }
        screenId = screens1.last();
    }


    // devices[currentDeviceIndex]->runInAdb("monkey -p " + Defs::KEY_PACKAGE_ID + "  -c android.intent.category.LAUNCHER 1", [this, pkgId, actyId](QProcess* p){
    // QString out = p->readAll();

    QMap<QString, QString> extras;
    extras.insert("package", pkgId);
    extras.insert("activity", actyId);
    // requestAction(Defs::KEY_LAUCH_ACTIVITY, extras, [this, pkgId, actyId](QProcess* p){
        // QString out = p->readAll();
    QString dpi = QString().setNum(currentDevice()->screenDpi().toDouble() / 2);
    // dpi = QString().setNum(QApplication::primaryScreen()->logicalDotsPerInch());

        currentDevice()->runInAdb("wm density "+dpi+" -d "+screenId);
        currentDevice()->runInAdb("am start --display " + screenId + " -n "+pkgId+"/"+actyId, [this, pkgId, screenId, actyId](QProcess* p){
            QString out = p->readAll();
            if (out.contains("Exception") || out.contains("Error")){
                currentDevice()->runInAdb("am start --display " + screenId + " -n "+Defs::KEY_PACKAGE_ID+"/.MainActivity"+
                                                // " -f 0x04000000"+
                                                " --es package "+ pkgId +
                                                " --es activity " + actyId +
                                                " --es display "+ screenId );
            }
        });
    // });
    // });
}

Device* MainWindow::currentDevice(){
    return devices[currentDeviceIndex];
}

void MainWindow::on_devices_activated(int index)
{
    on_devices_currentIndexChanged(index);
}

