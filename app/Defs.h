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
    static  QString KEY_GET_LABEL;
    static  QString KEY_LAUNCHERS_FILE;

    static  QString KEY_SAVED_PACKAGE_ICON;

    static  QString KEY_RUN_PACKAGE;

    static QString KEY_START_SERVICE;
    static QString KEY_STOP_SERVICE;

    static QString KEY_LAUCH_ACTIVITY;

    static QString KEY_APP_RESUMED;

    static  QString KEY_GET_NOTIFICATIONS_STATUS;
    static  QString KEY_GET_REQUEST_NOTIFICATIONS_ACCESS;

    static  QString remoteFupDir();
    static  QString remoteFupIconsDir();

    static QString getPackagesKey();

    static QString actionHeader(QString);

    static QString localIconsPath(const QString id);
    static QString localPath(const QString id);
};

#endif // DEFS_H
