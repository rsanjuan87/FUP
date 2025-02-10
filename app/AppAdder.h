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
    AppAdder(Config *config, QString deviceId, QString jsonFilePath, QObject* parent);

    QString filePath;
    QSet<LauncherInfo*> launchers;
    // void setList(QStringList out){
    //     this->out = out;
    // }
    bool kill = false;
    void stop();;

    void processStrJson(QString str);

    void saveIcon(const QString pkgId, const QByteArray body);


    bool checkIcon(const QString pkgId, const int size);


    static QMap<QString, QString> toJson(const QString& jsonString);



    static QList<QMap<QString, QString>> convertirJsonAListaDeMapas(const QString &jsonString);


    void getIcon(QString remotePath);

signals:
    void addLauncher(LauncherInfo*);
    void launchersClearred();
    void launchersSet(QSet<LauncherInfo*>);

    void deviceDiconected();

protected:
    void run() override;;

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


};




#endif // APPADDER_H
