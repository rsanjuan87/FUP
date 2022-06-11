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
#include "Defs.h"
#include "config.h"
#include "notificationhelper.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MouseButtonSignaler : public QObject {
  Q_OBJECT
  bool eventFilter(QObject * obj, QEvent * ev) Q_DECL_OVERRIDE {
    if (obj->isWidgetType())
      emit mouseButtonEvent(static_cast<QWidget*>(obj),
                            static_cast<QMouseEvent*>(ev));
    return false;
  }
public:
  Q_SIGNAL void mouseButtonEvent(QWidget *, QMouseEvent *);
  MouseButtonSignaler(QObject * parent = 0) : QObject(parent) {}
  void installOn(QWidget * widget) {
    widget->installEventFilter(this);
  }
};

class AppAdder : public QThread{
    Q_OBJECT
    Config *config;
    MouseButtonSignaler *signaler;
    QStringList out;
public:
    AppAdder(Config *config, MouseButtonSignaler *signaler){
        this->config = config;
        this->signaler = signaler;
    }
    void setList(QStringList out){
        this->out = out;
    }

signals:
    void addApp(QString id);

protected:
    void run() override{
        updatePackages(out);
        this->deleteLater();
    };


    void updatePackages(QStringList list){
        //starts with package:
        for(int i= 0; i < list.size(); i++){
            QString s = list.at(i);
            QString id = s.replace("package:", "");
            if(s.isEmpty()) continue;

            QProcess a;
            a.start(config->getAdbPath() + " shell \"cmd package resolve-activity --brief "+id+" \"");
            a.waitForFinished();
            QString out = QString(a.readAllStandardOutput());
            if(out.contains("No activity found")) continue;

            emit addApp(id);
        }
    }
};




class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QSystemTrayIcon *tray, QMenu *trayMenu, QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void requestAction(QString action);
    void connectDevice();
protected slots:
    void getAppList();
    void init();
    void readAdb(int);
    void readLogcat();
    void runInAdb(QString shell, QString key = "");

    void parse(QStringList out);
    void parseMessages(QStringList out);
    //void updatePackages(QStringList list);
    void onGeneralClick(QWidget *w, QMouseEvent *e);
    void addApp(QString id);
private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QProcess adb;
    QProcess logcat;
    QProcess scrcpy;
    Config config;
    QSystemTrayIcon *tray;
    MouseButtonSignaler signaler;
    AppAdder *appAdder;
    NotificationHelper *notifHelper;
    QMenu *trayMenu;
};



#endif // MAINWINDOW_H
