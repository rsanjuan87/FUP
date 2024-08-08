#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Defs.h"
#include "configdialog.h"

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

bool isDarkTheme() {
    QPalette palette = qApp->palette();
    QColor backgroundColor = palette.color(QPalette::Window);
    int brightness = qGray(backgroundColor.rgb());
    return brightness < 128;
}

MainWindow::MainWindow(QSystemTrayIcon *t, QMenu *trayMenu, QWidget *parent)    :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    tray(t),
    trayMenu(trayMenu),
    appsLayout(new WrapLayout(0))

{
    ui->setupUi(this);
    ui->toolBar->insertWidget(ui->actionCast_main_screen, ui->devices);
    init();
    connect(this, &MainWindow::reloadDevices, this, &MainWindow::loadDevices);

    if(isDarkTheme()){
        ui->actionReload_list->setIcon(QIcon(":/refresh-light"));
        ui->actionSound->setIcon(QIcon(":/imgs/audio-on-light"));
    }

    //getAppList();
    requestAction(Defs::KEY_GET_LAUNCHERS);

    ui->appListLayout->insertLayout(0, appsLayout);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete devicesManager;
    tray->hide();
}

void MainWindow::init(){

    trayMenu->addAction("Reconectar", this, &MainWindow::connectCurrentDevice);
    trayMenu->addSeparator();
    trayMenu->addAction("Mostrar Lanzador", this, SLOT(show()));
    trayMenu->addSeparator();
    trayMenu->addAction("Salir", qApp, SLOT(quit()));

    trayMenu->setDefaultAction(trayMenu->actions().first());

    devicesManager = new DevicesManager(&config, tray, this);
    connect(devicesManager, &DevicesManager::deviceAdded, this, &MainWindow::addDevice);
    connect(devicesManager, &DevicesManager::deviceUpdated, this, &MainWindow::addDevice);
    connect(devicesManager, &DevicesManager::deviceRemoved, this, &MainWindow::removeDevice);

    connect(devicesManager, &DevicesManager::addLauncher, this, &MainWindow::addLauncherSlot);
    connect(devicesManager, &DevicesManager::launchersClearred, this, &MainWindow::launchersClearredSlot);
    connect(devicesManager, &DevicesManager::launchersSet, this, &MainWindow::launchersSetSlot);


    loadDevices();
}

void MainWindow::loadDevices(){
    devicesManager->load();
}

void MainWindow::addDevice(Device* dev){
    QIcon icon(":/imgs/status/"+dev->status());
    for (int i = 0; i < ui->devices->count(); i++) {
        if(ui->devices->itemData(i).toString() == dev->id()){
            ui->devices->setItemText(i, dev->model());
            ui->devices->setItemIcon(i, icon);
            return;
        }
    }
    ui->devices->addItem(icon, dev->model(), dev->id());
}

void MainWindow::removeDevice(QString id){
    int c = ui->devices->count();
    for (int i = 0; i < c; ++i) {
        QString _id = ui->devices->itemData(i).toString();
        if(_id == id){
            ui->devices->removeItem(i);
            devicesManager->deviceRemovedSlot(id);
            break;
        }
    }

    if(currentDeviceId == id){
        if (c > 0){
            currentDeviceId = ui->devices->itemData(0).toString();
        }else{
            currentDeviceId = "";
        }
    }
}

// void MainWindow::loadDeviceslist(QStringList lines){
//     // if (lastDevices == lines){
//     //     return;
//     // }
//     // lastDevices = lines;
//     // ui->devices->clear();
//     // devices.clear();

//     foreach (QString l, lines) {
//         if(l.contains("device ") || l.contains("offline ")){
//             Device* dev = new Device(l, &config, tray);
//             devices.insert(dev->id(), dev);
//             QString status = "";
//             if(l.contains("offline")){
//                 status = ":off";
//             }
//             ui->devices->addItem(dev->id()+status, dev->id());
//         }
//     }
//     if (currentDeviceId.isEmpty() && !devices.isEmpty() ){
//         currentDeviceId = devices.firstKey();
//         on_devices_currentIndexChanged(0);
//     }else{
//         loadLaunchers();
//     }
//     disconnect(deviceListLoader, SIGNAL(devices(QStringList)), this, SLOT(loadDeviceslist(QStringList)));
//     delete deviceListLoader;

//     // connectCurrentDevice();
// }

void MainWindow::connectCurrentDevice()
{
    if(!currentDeviceId.isEmpty() && !devicesManager->isEmpty() ){
        devicesManager->value(currentDeviceId)->connectDevice();
        // reloadLaunchers();
    }
}


void MainWindow::requestAction(QString action, std::function<void(QProcess*)> onAdbFinished){
    if(currentDeviceId.isEmpty() && !devicesManager->isEmpty() ){
        return devicesManager->value(currentDeviceId)->requestAction(action, onAdbFinished);
    }
}

void MainWindow::requestAction(QString action, QMap<QString, QString> extras, std::function<void(QProcess*)> onAdbFinished) {
    if(currentDeviceId.isEmpty() && !devicesManager->isEmpty() ){
        return devicesManager->value(currentDeviceId)->requestAction(action, extras, onAdbFinished);
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


void MainWindow::launchersClearredSlot(QString id){
    if (currentDeviceId != "" && currentDevice() != nullptr && currentDevice()->id() != id){
        return;
    }
    // launchers.clear();
    while (QLayoutItem *item = appsLayout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->hide();
            // widget->deleteLater();
            delete widget;
        }
        delete item;
    }
    // ui->appListLayout->layout()->removeItem(appsLayout);
    // delete appsLayout;
}

void MainWindow::on_devices_currentIndexChanged(int index)
{
    // if (!currentDeviceId.isEmpty()){
    //     Device* dev = currentDevice();
    //     if(dev != nullptr){
    //         disconnect(dev, SIGNAL(addLauncher(LauncherInfo*,QString)));
    //         disconnect(dev, SIGNAL(launchersClearred(QString)));
    //     }
    // }
    QString id = ui->devices->itemData(index).toString();
    currentDeviceId = id;
    connectCurrentDevice();
    reloadLaunchers();

}


void MainWindow::reloadLaunchers(){
    launchersClearredSlot(currentDeviceId);
    loadLaunchers();
}

void MainWindow::loadLaunchers(){
    if(currentDeviceId.isEmpty()){
        return;
    }
    // appsLayout = new WrapLayout(0);
    // ui->appListLayout->insertLayout(0, appsLayout);
    connect(currentDevice(), &Device::addLauncher, this, &MainWindow::addLauncherSlot);
    connect(currentDevice(), &Device::launchersClearred, this, &MainWindow::launchersClearredSlot);
    connect(currentDevice(), &Device::deviceDisconected, this, &MainWindow::diconectDevice);
    connect(currentDevice(), &Device::appClosed, this, &MainWindow::appClosedSlot);
    launchersSetSlot(currentDevice()->launchers(), currentDeviceId);
}

void MainWindow::appClosedSlot(QString pkgid){
    if(pkgid == Defs::ActionMainScreen){
        ui->actionCast_main_screen->setChecked(false);
    }else if(pkgid == Defs::ActionVirtualDesktop){
        ui->actionCast_virtual_desktop_size_screen->setChecked(false);
    }else{
        currentDevice()->stopScrcpy(pkgid);
    }
}

void MainWindow::launchersSetSlot(QSet<LauncherInfo*> launchers, QString deviceId){
    if(deviceId != currentDevice()->id()){
        return;
    }
    foreach (LauncherInfo *info, launchers) {
        addLauncherSlot(info, deviceId);
    }
}

void MainWindow::addLauncherSlot(LauncherInfo* info, QString id){
    QString iconPath = Defs::localIconsPath(id)+info->pkgId;
    QIcon icon(iconPath);
    if(id != currentDevice()->id()){
        return;
    }
    for (int i = 0; i < appsLayout->count(); i++) {
        auto item = appsLayout->itemAt(i);
        QWidget* btn = item->widget();
        if (btn->property(Defs::KEY_GET_PACKAGES.toUtf8()) == info->pkgId &&
            btn->property(Defs::KEY_GET_LAUNCHERS.toUtf8()) == info->activityId &&
            btn->property(Defs::KEY_GET_LABEL.toUtf8()) == info->label) {
            return;
        }
    }
    // if(launchers.contains(info))return;
    // launchers.insert(info);
    QToolButton *btn = new QToolButton(0);
    btn->setIcon(icon);
    btn->setIconSize(QSize(56, 56));
    btn->setText(info->label);
    btn->setProperty(Defs::KEY_GET_PACKAGES.toUtf8(), info->pkgId);
    btn->setProperty(Defs::KEY_GET_LAUNCHERS.toUtf8(), info->activityId);
    btn->setProperty(Defs::KEY_GET_LABEL.toUtf8(), info->label);
    btn->setProperty("deviceId", id);
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn->setAutoRaise(false);
    btn->setFixedWidth(70);
    btn->setToolTip(info->label);
    connect(btn, &QToolButton::clicked, this, [this, btn]() { onAppClick(btn); });
    btn->setFocusPolicy(Qt::TabFocus);
    appsLayout->addWidget(btn);
}


void MainWindow::onAppClick(QWidget *w){

    if(currentDeviceId.isEmpty() || devicesManager->isEmpty() ){
        return;
    }


    QString pkgId = w->property(Defs::KEY_GET_PACKAGES.toUtf8()).toString();
    QString actyId = w->property(Defs::KEY_GET_LAUNCHERS.toUtf8()).toString();
    QString actyLabel = w->property(Defs::KEY_GET_LABEL.toUtf8()).toString();
    QString deviceId = w->property("deviceId").toString();
    if(deviceId != currentDeviceId){
        return;
    }
    QString id = pkgId;
    QString label = actyLabel;
    if(config.coherenceMode){
        label = actyLabel;
    }else{
        label = currentDevice()->model();
        id = Defs::ActionMainScreen;
    }

    try{
        QProcess::ProcessState status = currentDevice()->scrcpyStatus(id);
        QString screenId = "0";
        if(status != QProcess::Running){
            currentDevice()->stopScrcpy(id);
            QStringList screens = currentDevice()->screens();
            currentDevice()->runScrcpy(id, label);
            QStringList screens1 = currentDevice()->screens();
            if (screens.length() == 0){
                QMessageBox::critical(this, "Error", "Error getting displays on "+currentDevice()->id());
                return;
            }
                while(screens.length() >= screens1.length() ){
                    screens1 = currentDevice()->screens();
                }
                screenId = screens1.last();
        }else{
            currentDevice()->raiseWindow(id, label);
            screenId = currentDevice()->screenId(id);
        }

        ///resend app to de window

        // devices[currentDeviceIndex]->runInAdb("monkey -p " + Defs::KEY_PACKAGE_ID + "  -c android.intent.category.LAUNCHER 1", [this, pkgId, actyId](QProcess* p){
        // QString out = QString(p.readAll()).remove("\r");

        QMap<QString, QString> extras;
        extras.insert("package", pkgId);
        extras.insert("activity", actyId);
        if(!config.coherenceMode){
            screenId = "0";
        }
        // requestAction(Defs::KEY_LAUCH_ACTIVITY, extras, [this, pkgId, actyId](QProcess* p){
        // QString out = QString(p.readAll()).remove("\r");
        currentDevice()->runInAdb("am start --display " + screenId + " -n "+pkgId+"/"+actyId, [this, pkgId, screenId, actyId](QProcess* p){
            QString out = QString(p->readAll()).remove("\r");
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
    }catch(QException e){
        qDebug() << e.what();
    }
}

void MainWindow::diconectDevice(QString id){
    devicesManager->diconectDevice(id);
    int index = 0;
    for (; index < ui->devices->count(); index++) {
        if(ui->devices->itemData(index) == id){
            ui->devices->removeItem(index);
            break;
        }
    }
    if(currentDeviceId == id){
        launchersClearredSlot(id);
    }
}

Device* MainWindow::currentDevice(){
    if(currentDeviceId.isEmpty()){
        return nullptr;
    }
    return devicesManager->value(currentDeviceId);
}

void MainWindow::on_devices_activated(int index)
{
    on_devices_currentIndexChanged(index);
}


void MainWindow::on_actionCast_main_screen_toggled(bool v)
{
    if(v){
        currentDevice()->runScrcpy(Defs::ActionMainScreen);
    }else{
        currentDevice()->stopScrcpy(Defs::ActionMainScreen);
    }
}



void MainWindow::on_actionCast_virtual_desktop_size_screen_toggled(bool v)
{
    if(v){
        currentDevice()->runScrcpy(Defs::ActionVirtualDesktop, "Desktop");
    }else{
        currentDevice()->stopScrcpy(Defs::ActionVirtualDesktop);
    }
}

void MainWindow::on_actionSound_toggled(bool v) {
    if (v) {
        currentDevice()->castAudioStart();
    } else {
        currentDevice()->castAudioStop();
    }
}

void MainWindow::on_actionClear_cache_triggered() {
    QDir().remove(Defs::localPath(""));
}

void MainWindow::requestLoadDevices() { emit reloadDevices(); }

void MainWindow::on_actionSettings_triggered() {
    ConfigDialog configDlg;
    if(configDlg.exec()){
        config.load();
    }
}

void MainWindow::on_actionExit_triggered()
{
    exit(0);
}

