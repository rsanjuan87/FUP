#ifndef DEFS_H
#define DEFS_H

#include <QStandardPaths>
#include <QString>


class Defs{
public:
    static  QString KEY_NOTIFICACTION_CHANNEL;
    static  QString KEY_PACKAGE_ID;

    static  QString KEY_GET_PACKAGES;

    static  QString KEY_GET_PACKAGE_INFO;
    static  QString KEY_GET_PACKAGE_ICON;

    static  QString KEY_GET_LAUNCHERS;
    static  QString KEY_SAVED_PACKAGE_ICON;

    static  QString KEY_RUN_PACKAGE;

    static  QString KEY_START_SERVICE;
    static  QString KEY_STOP_SERVICE;

    static  QString KEY_LAUCH_ACTIVITY;

    static  QString KEY_GET_NOTIFICATIONS_STATUS;
    static  QString KEY_GET_REQUEST_NOTIFICATIONS_ACCESS;

    static  QString KEY_FUP_DIR;
    static  QString KEY_FUP_ICONS_DIR;

    static QString getPackagesKey();

    static QString actionHeader(QString);

    static QString iconsPath();
};

#endif // DEFS_H
