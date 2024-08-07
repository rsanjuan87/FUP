#include "mainwindow.h"

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>

#include "models/launcherinfo.h"

int main(int argc, char *argv[])
{
    QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << ":/imgs/elementary_icons");
    qRegisterMetaType<QList<LauncherInfo*>>("QList<LauncherInfo*>");
    qRegisterMetaType<QSet<LauncherInfo*>>("QSet<LauncherInfo*>");
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");

    QApplication a(argc, argv);
    QSystemTrayIcon tray;
    tray.setIcon(QIcon(":tray_light"));
    tray.show();
#ifdef Q_OS_MAC
    qApp->setWindowIcon(QIcon(":/imgs/icon_mac"));
#else
    qApp->setWindowIcon(QIcon(":/imgs/icon"));
#endif

    QMenu menu;
    tray.setContextMenu(&menu);
    MainWindow w(&tray, &menu);
    w.show();
    a.setQuitOnLastWindowClosed(false);
    return a.exec();
}
