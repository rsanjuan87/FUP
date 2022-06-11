#ifndef NOTIFICATIONHELPER_H
#define NOTIFICATIONHELPER_H

#include <QString>
#include <QSystemTrayIcon>



class NotificationHelper
{
public:
    NotificationHelper(QSystemTrayIcon *t);

    void parse(QString data);

private:
    QSystemTrayIcon *tray;
    QHash<QString, QString> notifications;
};

#endif // NOTIFICATIONHELPER_H
