#include "Defs.h"
#include <QFileInfo>
#include <QApplication>
#include <QJsonDocument>
#include "AppAdder.h"
#include "notificationhelper.h"

NotificationHelper::NotificationHelper(QString devId, QSystemTrayIcon *t, Config *config, QString remoteIconPath)
    :
    id(devId),
    remoteIconPath(remoteIconPath),
    tray(t),
    config(config)
{

}

void NotificationHelper::parse(QString data, QString deviceId)
{

    QList<QMap<QString, QString>> list = AppAdder::convertirJsonAListaDeMapas(data);

    for( QMap<QString, QString> it : list){
        QString id = it["NID"];
        if(it["status"] == "posted"){
            //if(!msg.isEmpty())

            QString iconPath = ":/imgs/tray/tray_dark";
            QString appId = it["PKGID"];
            QString icon = it["icon"];
            getIcon(icon);
            if(QFileInfo::exists(Defs::localIconsPath(deviceId)+icon))
                iconPath = Defs::localIconsPath(deviceId)+icon;
            else if(QFileInfo::exists(Defs::localIconsPath(deviceId)+appId))
                iconPath = Defs::localIconsPath(deviceId)+appId;

            QString msg = it["MSG"];
            QString appName = it["PKGNAME"];
            tray->showMessage(appName, msg, QIcon(iconPath));

            notifications.insert(id, data);
        }else{
            notifications.remove(id);
            //TODO should remove notification
        }
    }
}




void NotificationHelper::getIcon(QString remotePath){
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    QString localFolderIconPath = Defs::localIconsPath(id);
    QDir().mkpath(localFolderIconPath);
    QFileInfo remoteInfo(remotePath);
    QFile file(localFolderIconPath + "/" + remoteInfo.fileName());
    //check if exists and same size in bits
    if(file.exists()){
        QStringList params;
        params << "-s" << id << "shell"
               << "run-as " + Defs::KEY_PACKAGE_ID +
                      " ls -la " + remotePath;
        p.start(config->adbPath(),params);
        p.waitForFinished();
        QString out = QString(p.readAll());
        while(out.contains("  ")){
            out = out.replace("  ", " ");
        }
        if(!out.isEmpty()){
            QStringList spl = out.split(" ");
            if(spl.length() >= 5){
                out = spl.at(4);
            }
        }
        qint64 sizeBits = out.toInt();
        if(sizeBits == file.size() && file.size() > 0){
            return;
        }else{
            file.remove();
        }
    }

    //load
    QStringList params;
    params << "-s" << id << "shell" << "run-as "+ Defs::KEY_PACKAGE_ID+" cat " + remotePath;
    // qDebug() << "get icon";
    p.start(config->adbPath(),params);
    p.waitForFinished();
    QByteArray  out = p.readAll();
#ifdef Q_OS_WIN
    out = out.replace("\r\n", "\n");
#endif

    if (file.open(QIODevice::WriteOnly)) {
        file.write(out);
        file.close();
    } else {
        // Manejar error de apertura de archivo
    }

}
