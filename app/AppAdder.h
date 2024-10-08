#ifndef APPADDER_H
#define APPADDER_H


#include <QtCore/qobjectdefs.h>
#include <QJsonDocument>
#include <QMap>
#include <QProcess>

#include <QJsonDocument>
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsonobject.h>
#include <QDir>
#include <QThread>

#include "models/launcherinfo.h"
#include "Defs.h"
#include "config.h"

class AppAdder : public QThread{
    Q_OBJECT
    Config *config;
    QStringList out;
    QString id;
public:
    AppAdder(Config *config, QString deviceId, QString jsonFilePath, QObject* parent)
        : QThread(parent)
    {
        this->config = config;
        this->id = deviceId;
        this->filePath = jsonFilePath;
    }

    QString filePath;
    QSet<LauncherInfo*> launchers;
    // void setList(QStringList out){
    //     this->out = out;
    // }
    bool kill = false;
    void stop() { kill = true; };

signals:
    void addLauncher(LauncherInfo*);
    void launchersClearred();
    void launchersSet(QSet<LauncherInfo*>);

    void deviceDiconected();

protected:
    void run() override{
        //getFile(Defs::KEY_LAUNCHERS_FILE);
        QProcess p;
        // p.setTextModeEnabled(true);
        p.setProcessChannelMode(QProcess::MergedChannels);
        QStringList params;
        params << "-s" << id << "shell" << "run-as"<<  Defs::KEY_PACKAGE_ID <<"cat" << filePath;
        p.start(config->adbPath(), params);
        p.waitForFinished();
        QString str = p.readAll();
        launchers.clear();
        emit launchersClearred();
        QList<QMap<QString, QString>> map = convertirJsonAListaDeMapas(str);
        QFileInfo ifn(filePath);
        QString remoteFupIconsDir = ifn.dir().path() + "/icons/";
        for (auto &&it : map) {
            if (kill) return;
            QString pckg = it["LAUNCHER_PKG"];
            QString label = it["LAUNCHER_LABEL"];
            QString launch = it["LAUNCHER_ACTY"];
            getIcon(remoteFupIconsDir + pckg);
            if (kill) return;
            LauncherInfo* info = new LauncherInfo(pckg, launch, label);
            //signaler.installOn(btn);
            launchers.insert(info);
            emit addLauncher(info);
            if (kill) return;
        }
        if(!launchers.isEmpty()){
            emit launchersSet(launchers);
        }
        p.terminate();
        // this->deleteLater();
    };


    // void updatePackages(QStringList list){
    //     //starts with package:
    //     for(int i= 0; i < list.size(); i++){
    //         QString s = list.at(i);
    //         QString id = s.replace("package:", "");
    //         if(s.isEmpty()) continue;

    //         QProcess a;
    //         a.start(config->adbPath(), QStringList(" shell \"cmd package resolve-activity --brief "+id+" \""));
    //         a.waitForFinished();
    //         QString out = QString(a.readAllStandardOutput());
    //         if(out.contains("No activity found")) continue;

    //         emit addApp(id);
    //     }
    // }



    QList<QMap<QString, QString>> convertirJsonAListaDeMapas(const QString &jsonString) {
        QList<QMap<QString, QString>> listaDeMapas;

        QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
        if (!doc.isArray()) {
            qDebug() << "JSON no es un array";
            qDebug() << jsonString;
            return listaDeMapas;
        }

        QJsonArray jsonArray = doc.array();
        for(const QJsonValue &jsonValue : jsonArray) {
            QMap<QString, QString> mapa;
            if (!jsonValue.isObject()) continue;
            QJsonObject jsonObject = jsonValue.toObject();
            QJsonObject::iterator it;
            for (it = jsonObject.begin(); it != jsonObject.end(); ++it) {
                QString clave = it.key();
                QJsonValue valor = it.value();
                mapa.insert(clave, valor.toString());
            }
            listaDeMapas.append(mapa);
        }

        return listaDeMapas;
    }

    void getIcon(QString remotePath){
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
        if(QString(out).contains("not found")){
            stop();
            emit deviceDiconected();
        }

        if (file.open(QIODevice::WriteOnly)) {
            file.write(out);
            file.close();
        } else {
            // Manejar error de apertura de archivo
        }

    }


};




#endif // APPADDER_H
