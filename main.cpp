#include "mainwindow.h"

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QSystemTrayIcon tray;
    tray.setIcon(QIcon(":/imgs/tray/tray_dark"));
    tray.show();
    qApp->setWindowIcon(QIcon(":/imgs/tray/tray_dark"));


    QMenu menu;
    tray.setContextMenu(&menu);
    MainWindow w(&tray, &menu);
    w.show();
    a.setQuitOnLastWindowClosed(false);
    return a.exec();
}
