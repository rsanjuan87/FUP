#include "mainwindow.h"

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QSystemTrayIcon tray;
    tray.setIcon(QIcon(":tray_light"));
    tray.show();
#ifdef Q_OS_MAC
    qApp->setWindowIcon(QIcon(":/imgs/icon_mac"));
#elif
    qApp->setWindowIcon(QIcon(":/imgs/icon"));
#endif

    QMenu menu;
    tray.setContextMenu(&menu);
    MainWindow w(&tray, &menu);
    w.show();
    a.setQuitOnLastWindowClosed(false);
    return a.exec();
}
