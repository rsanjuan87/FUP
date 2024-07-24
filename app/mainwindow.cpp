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
    loadDeviceslist();

    trayMenu->addAction("Reconectar", this, &MainWindow::connectDevice);
    trayMenu->addSeparator();
    trayMenu->addAction("Mostrar Lanzador", this, SLOT(show()));
    trayMenu->addSeparator();
    trayMenu->addAction("Salir", qApp, SLOT(quit()));

    connectDevice();
    trayMenu->setDefaultAction(trayMenu->actions().first());

}

void MainWindow::loadDeviceslist(){
    if(!devices.empty()){
        devices.clear();
    }
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.start(config.getAdbPath(), QString("devices -l").split(" "));
    p.waitForFinished();
    QString out = p.readAll();
    QStringList lines = out.split("\n");

    foreach (QString l, lines) {
        while (l.contains("  ")) {
            l = l.replace("  ", " ");
        }
        l = l.trimmed();
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
        connectDevice();
    }
}

void MainWindow::connectDevice()
{
    if(currentDeviceIndex != -1 && devices.isEmpty() ){
        return devices[currentDeviceIndex]->connectDevice();
    }
}


void MainWindow::requestAction(QString action){
    if(currentDeviceIndex != -1 && devices.isEmpty() ){
        return devices[currentDeviceIndex]->requestAction(action);
    }
}

void MainWindow::requestAction(QString action, QMap<QString, QString> extras) {
    if(currentDeviceIndex != -1 && devices.isEmpty() ){
        return devices[currentDeviceIndex]->requestAction(action, extras);
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
    requestAction(Defs::KEY_GET_LAUNCHERS);
}


void MainWindow::on_devices_currentIndexChanged(int index)
{

    int count = appsLayout->count();
    for (int i = 0; i < count; i++){
        auto item = appsLayout->itemAt(0);
        appsLayout->removeItem(item);
    }

    currentDeviceIndex = index;
    connectDevice();
    Device* device = devices[index];
    count = device->launchers.size();
    for (int j = 0; j < count; j++) {

        LauncherInfo info(device->launchers.at(j));

        QIcon icon(Defs::iconsPath()+info.pkgId);
        QToolButton *btn = new QToolButton(0);
        btn->setIcon(icon);
        btn->setIconSize(QSize(56, 56));
        btn->setText(info.label);
        btn->setProperty(Defs::KEY_GET_PACKAGES.toUtf8(), info.pkgId);
        btn->setProperty(Defs::KEY_GET_LAUNCHERS.toUtf8(), info.activityId);
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn->setAutoRaise(false);
        btn->setStyleSheet("background-color: rgba(0, 230, 255, 0);");
        btn->setFixedWidth(70);
        connect(btn, &QToolButton::clicked, this, [this, btn]() { onAppClick(btn); });
        appsLayout->addWidget(btn);

    }
}


void MainWindow::onAppClick(QWidget *w){

    if(currentDeviceIndex == -1 || devices.isEmpty() ){
        return;
    }

    QString pkgId = w->property(Defs::KEY_GET_PACKAGES.toUtf8()).toString();
    QString actyId = w->property(Defs::KEY_GET_LAUNCHERS.toUtf8()).toString();


    if(actyId.isEmpty()){
        devices[currentDeviceIndex]->runInAdb("monkey -p " + pkgId + "  -c android.intent.category.LAUNCHER 1");
    }else{
        if(!config.coherenceMode){
            devices[currentDeviceIndex]->runInAdb("am start -n "+pkgId+"/"+actyId);
        }else{
            QMap<QString, QString> extras;
            extras.insert("package", pkgId);
            extras.insert("activity", actyId);
            requestAction(Defs::KEY_LAUCH_ACTIVITY, extras);
        }
    }
    if(!config.coherenceMode){
        devices[currentDeviceIndex]->stopScrcpy();
    }
    devices[currentDeviceIndex]->runScrcpy();
}
