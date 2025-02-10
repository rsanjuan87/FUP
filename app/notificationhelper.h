#ifndef NOTIFICATIONHELPER_H
#define NOTIFICATIONHELPER_H

#include "config.h"

#include <QHash>
#include <QString>
#include <QSystemTrayIcon>

class NotificationHelper
{
public:
    NotificationHelper(QString devId, QSystemTrayIcon *t, Config* config, QString remoteIconPath);

    void parse(QString data, QString deviceId);

protected slots:
    void getIcon(QString remotePath);
private:
    QString id;
    QString remoteIconPath;
    QSystemTrayIcon *tray;
    Config *config;
    QHash<QString, QString> notifications;
};

#endif // NOTIFICATIONHELPER_H
