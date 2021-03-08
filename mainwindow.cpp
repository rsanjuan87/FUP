#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Defs.h"

#include <QFileInfo>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    init();

    getAppList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init(){
      connect(&signaler, SIGNAL(mouseButtonEvent(QWidget *, QMouseEvent *)),
              this, SLOT(onGeneralClick(QWidget *, QMouseEvent *)));

    adb.setTextModeEnabled(true);
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
        runInAdb("monkey -p " + id + "  -c android.intent.category.LAUNCHER 1", Defs::actionHeader(Defs::KEY_RUN_PACKAGE));
        scrcpy.kill();
        scrcpy.start(config.getScrcpyPath());
    }

}









