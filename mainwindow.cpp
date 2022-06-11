#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "notificationhelper.h"
#include "Defs.h"

#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>

MainWindow::MainWindow(QSystemTrayIcon *t, QMenu *trayMenu, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , tray(t)
    , notifHelper(new NotificationHelper(t))
    , trayMenu(trayMenu)
{
    ui->setupUi(this);

    init();

    getAppList();
}

MainWindow::~MainWindow()
{
    delete ui;
    adb.terminate();
    scrcpy.terminate();
    logcat.terminate();
    tray->hide();
}

void MainWindow::connectDevice(){

    logcat.terminate();

    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);

    /////creating remote folders
    p.start(config.getAdbPath() +" mkdir /data/local/tmp/fup/icons");
    p.waitForFinished();
    p.start(config.getAdbPath() +" chmod 777 /data/local/tmp/fup/");
    p.waitForFinished();
    p.start(config.getAdbPath() +" chmod 777 /data/local/tmp/fup/icons");
    p.waitForFinished();

    ///clean p readstream
    p.readAll();

    //get remote date
    p.start(config.getAdbPath() , QStringList() << "shell" <<"date +'%m-%d %H:%M:%S.000'");
    p.waitForFinished();
    QString time = p.readAll().replace("\n", "");
    //to connnect logcat with that date
    logcat.start(config.getAdbPath() +" shell logcat -s "+ Defs::KEY_PACKAGE_ID + " -T '"+
                 time+"'");

    requestAction(Defs::KEY_START_SERVICE);

}

void MainWindow::init(){

    ///connections
      connect(&signaler, SIGNAL(mouseButtonEvent(QWidget *, QMouseEvent *)),
              this, SLOT(onGeneralClick(QWidget *, QMouseEvent *)));

    adb.setProcessChannelMode(QProcess::MergedChannels);
    connect(&adb, SIGNAL(finished(int)), this, SLOT(readAdb(int)));

    scrcpy.setProgram(config.getScrcpyPath());
    QProcessEnvironment env = scrcpy.processEnvironment();
    env.insert("PATH", env.value("PATH")+":"+QFileInfo(config.getAdbPath()).dir().path());
    scrcpy.setProcessEnvironment(env);

    appAdder = new AppAdder(&config, &signaler);
    for (int i = 0; i > ui->appListLayout->count(); i++) {
        ui->appListLayout->removeWidget(ui->appListLayout->takeAt(0)->widget());
    }
    connect(appAdder, SIGNAL(addApp(QString)), this, SLOT(addApp(QString)));


    logcat.setProcessChannelMode(QProcess::MergedChannels);
    connect(&logcat, SIGNAL(readyRead()), this, SLOT(readLogcat()));
    connect(&logcat, SIGNAL(readyReadStandardError()), this, SLOT(readLogcat()));
    connect(&logcat, SIGNAL(readyReadStandardOutput()), this, SLOT(readLogcat()));

    trayMenu->addAction("Reconectar", this, &MainWindow::connectDevice);
    trayMenu->addSeparator();
    trayMenu->addAction("Mostrar Lanzador", this, SLOT(show()));
    trayMenu->addSeparator();
    trayMenu->addAction("Salir", qApp, SLOT(quit()));

    connectDevice();
    trayMenu->setDefaultAction(trayMenu->actions().first());
}

void MainWindow::requestAction(QString action){
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.startDetached(config.getAdbPath() , QStringList() << "shell"<<"am"<<"broadcast"<<"-a"<<"android.intent.action.VIEW"<<"-n"<<"org.santech.notifyme/org.santech.notifyme.Broadcaster"<<"--es"<<"action"<<"'"+action+"'");
}

void MainWindow::addApp(QString id){
    QPushButton *btn = new QPushButton(id, 0);
    btn->setProperty(Defs::KEY_GET_PACKAGES.toUtf8(), id);
    signaler.installOn(btn);
    ui->appListLayout->insertWidget(ui->appListLayout->count()-1,btn);
}


void MainWindow::getAppList(){
    runInAdb(" pm list packages -3", Defs::getPackagesKey());
}

void MainWindow::readAdb(int){
    QStringList out = QString(adb.readAll()).split('\n');
    parse(out);
}

void MainWindow::readLogcat(){
    QStringList out = QString(logcat.readAll()).split('\n');
    parseMessages(out);
}

void MainWindow::parseMessages(QStringList out){
    if(!out.isEmpty()){
        if(out.at(0).startsWith("---------"))
            out.removeAt(0);
        foreach(QString s, out){
            if(s.isEmpty())
                continue;
            int i = s.indexOf(Defs::KEY_PACKAGE_ID);
            QString data = s.mid(i+Defs::KEY_PACKAGE_ID.length()+2);
            if(data.startsWith(Defs::KEY_NOTIFICACTION_CHANNEL)){
                notifHelper->parse(data);
            }else
                if(data.startsWith(Defs::KEY_GET_PACKAGE_ICON)){
                    QProcess p;
                    QString path = data.remove(Defs::KEY_GET_PACKAGE_ICON+", ");
                    p.start(config.getAdbPath(), QStringList() << "shell"<<"cat"<<path<<" || "<<"echo"<<"''"<<">"<<path);
                    p.waitForFinished();
                    qDebug() << p.readAll();
            }else
                if(data.startsWith(Defs::KEY_SAVED_PACKAGE_ICON)){
                    QProcess p;
                    QString path = data.remove(Defs::KEY_SAVED_PACKAGE_ICON+", ");
                    QString local = Defs::iconsPath();
                    QDir().mkpath(local);
                    QFileInfo info(path);
                    p.start(config.getAdbPath()+" pull "+path+" "+local+"/"+info.fileName());
                    p.waitForFinished();
                    qDebug() << p.readAll();
                    qDebug() << local+"/"+info.fileName();
                }


        }

    }
}


void MainWindow::runInAdb(QString shell, QString key){
    adb.start(config.getAdbPath() +" shell "+ key +" "+shell);
}

void MainWindow::parse(QStringList out){
    if(!out.isEmpty()){
        if(out.at(0) == Defs::KEY_GET_PACKAGES){
            out.removeAt(0);
            //updatePackages(out);
            appAdder->setList(out);
            appAdder->start();
        }
    }
}

void MainWindow::onGeneralClick(QWidget *w, QMouseEvent *e){
    if(e->type() == QEvent::MouseButtonPress) {
        QString id = w->property(Defs::KEY_GET_PACKAGES.toUtf8()).toString();
        qDebug() << "Running package: "<< id;
        //shell am start -a android.settings.ACTION_NOTIFICATION_LISTENER_SETTINGS

        runInAdb("monkey -p " + id + "  -c android.intent.category.LAUNCHER 1", Defs::actionHeader(Defs::KEY_RUN_PACKAGE));
        scrcpy.kill();
        scrcpy.start(config.getScrcpyPath());
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
//adb shell 'am broadcast -a android.intent.action.VIEW -n org.santech.notifyme/org.santech.notifyme.Broadcaster --es sms_body "test from adb"'






void MainWindow::on_pushButton_clicked()
{
    requestAction(Defs::KEY_GET_LAUNCHERS);
}

