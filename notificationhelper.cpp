#include "notificationhelper.h"
#include "Defs.h"
#include <QFileInfo>
#include <QApplication>

NotificationHelper::NotificationHelper(QSystemTrayIcon *t)
    : tray(t)
{

}

void NotificationHelper::parse(QString data)
{
    QString d = data.remove(Defs::KEY_NOTIFICACTION_CHANNEL+", ");
    QString key = "status";
    int i = d.indexOf("\""+key+"\": \"")+key.length()+5;
    int o = d.indexOf("\" \t", i);
    QString status = d.mid(i, o-i);
    key = "NID";
    i = d.indexOf("\""+key+"\": \"")+key.length()+5;
    o = d.indexOf("\" \t", i);
    QString id = d.mid(i,  o-i);
    key = "MSG";
    i = d.indexOf("\""+key+"\": \"")+key.length()+5;
    o = d.indexOf("\" \t", i);
    QString msg = d.mid(i, o-i);
    key = "PKGID";
    i = d.indexOf("\""+key+"\": \"")+key.length()+5;
    o = d.indexOf("\" \t", i);
    QString appId = d.mid(i, o-i);
    key = "PKGNAME";
    i = d.indexOf("\""+key+"\": \"")+key.length()+5;
    o = d.indexOf("\" \t", i);
    QString appName = d.mid(i, o-i);
    if(status == "posted"){
        //if(!msg.isEmpty())
        QString iconPath = ":/imgs/tray/tray_dark";
             if(QFileInfo::exists(Defs::iconsPath()+appId))
                 iconPath = Defs::iconsPath()+appId;
             //qApp->setWindowIcon(QIcon(iconPath));

             tray->showMessage(appName,msg, QIcon(iconPath));

            //qApp->setWindowIcon(QIcon(":/imgs/tray/tray_dark"));
        notifications.insert(id, data);
    }else{
        notifications.remove(id);
        //TODO should remove notification
    }
}
