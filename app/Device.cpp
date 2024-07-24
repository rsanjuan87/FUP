
#include "Defs.h"
#include "Device.h"

#include <QJsonDocument>
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsonobject.h>

Device::Device(QString line, Config *conf, QSystemTrayIcon *t, QObject *parent)
    : QObject{parent},
     notifHelper(new NotificationHelper(t))
{
    this->line = line;
    this->config = conf;
    init();
}

void Device::init(){
    adb.setProcessChannelMode(QProcess::MergedChannels);
    connect(&adb, SIGNAL(finished(int)), this, SLOT(readAdb(int)));

    scrcpy.setProgram(config->getScrcpyPath());
    QProcessEnvironment env = scrcpy.processEnvironment();
    env.insert("PATH", env.value("PATH")+":"+QFileInfo(config->getAdbPath()).dir().path());
    scrcpy.setProcessEnvironment(env);


    logcat.setProcessChannelMode(QProcess::MergedChannels);
    connect(&logcat, &QIODevice::readyRead, this, &Device::readLogcat);
    connect(&logcat, &QProcess::readyReadStandardError, this, &Device::readLogcat);
    connect(&logcat, &QProcess::readyReadStandardOutput, this, &Device::readLogcat);

}


Device::~Device(){
    adb.terminate();
    scrcpy.terminate();
    logcat.terminate();
}

QString Device::id(){
    return line.split(" ").first();
}


QString Device::getScreenSize(){
    if(screenSize == NULL){
        QProcess devices;
        devices.setProcessChannelMode(QProcess::MergedChannels);
        devices.start(config->getAdbPath(),
                      QString("shell -s " + id() + " wm size").split(" "));
        devices.waitForFinished();
        QString out = devices.readAll();
        QStringList lines = out.split("\n");
        screenSize = new QString(out.split(": ").last().remove("\n"));
    }
    return *screenSize;
}

void Device::connectDevice(){

    logcat.terminate();

    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);

    /////creating remote folders
    QString cmd = "-s "+ id() +" shell mkdir -p " + Defs::KEY_FUP_ICONS_DIR;
    p.start(config->getAdbPath(),cmd.split(" "));
    p.waitForFinished();
    QString out = p.readAll();

    cmd = "-s "+ id() +" shell chmod 777 -R " + Defs::KEY_FUP_DIR;
    p.start(config->getAdbPath(), cmd.split(" "));
    p.waitForFinished();
    out = p.readAll();

    //clean p readstream
    p.readAll();

    //get remote date
    p.start(config->getAdbPath() , QString("-s "+ id() +" shell date +'%m-%d %H:%M:%S.000'").split(" "));
    p.waitForFinished();
    QString time = p.readAll().replace("\n", "");
    //to connnect logcat with that date
    logcat.start(config->getAdbPath(), QString("-s "+ id() +" shell logcat -s "+ Defs::KEY_PACKAGE_ID + " -T '"+time+"'").split(" "));

    requestAction(Defs::KEY_START_SERVICE);
    requestAction(Defs::KEY_GET_LAUNCHERS);

}

void Device::requestAction(QString action){
    return requestAction(action, QMap<QString, QString>());
}

void Device::requestAction(QString action, QMap<QString, QString> extras) {
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    QStringList params = QStringList()
                         << "-s" << id() <<"shell" << "am" << "broadcast" << "-a"
                         << "android.intent.action.VIEW" << "-n"
                         << Defs::KEY_PACKAGE_ID + "/" +
                                Defs::KEY_PACKAGE_ID + ".BroadcastReceiver"
                         << "--es" << "action" << "'" + action + "'";
    for (auto it = extras.constBegin(); it != extras.constEnd(); ++it) {
        params << "--es" << it.key() << it.value();
    }
    p.start(config->getAdbPath(), params);
    p.waitForFinished();
    QString a = p.readAll();
    qDebug() << a;
}



void Device::getAppList(){
    runInAdb("pm list packages -3");
}

void Device::readAdb(int){
    QStringList out = QString(adb.readAll()).split('\n');
    parse(out);
}

void Device::readLogcat(){
    QStringList out = QString(logcat.readAll()).split('\n');
    parseMessages(out);
}

QString Device::runInAdb(QString shell){
    adb.start(config->getAdbPath(), ("shell -s "+ id() +" "+shell).split(" "));

    adb.waitForFinished();
    QString out = adb.readAllStandardOutput()+"\n"+adb.readAllStandardError();
    return out;
}

void Device::parseMessages(QStringList out){
    //qDebug() << out;
    if(!out.isEmpty()){
        if(out.at(0).startsWith("---------"))
            out.removeAt(0);
        foreach(QString s, out){
            if(s.isEmpty())
                continue;
            int i = s.indexOf(Defs::KEY_PACKAGE_ID);
            QString data = s.mid(i+Defs::KEY_PACKAGE_ID.length()+2);
            if(data.startsWith(Defs::KEY_NOTIFICACTION_CHANNEL)){
                notifHelper->parse(data);
            } else if (data.startsWith(Defs::KEY_GET_PACKAGE_ICON)) {
                QProcess p;
                QString path = data.remove(Defs::KEY_GET_PACKAGE_ICON + ", ");
                p.start(config->getAdbPath(), QStringList()
                                                 << "shell"
                                                 << "cat" << path << " || "
                                                 << "echo"
                                                 << "''"
                                                 << ">" << path);
                p.waitForFinished();
                p.readAll();
            } else if (data.startsWith(Defs::KEY_SAVED_PACKAGE_ICON)) {
                QString remotePath = data.remove(Defs::KEY_SAVED_PACKAGE_ICON + ", ");
                getIcon(remotePath);
            }else if (data.startsWith(Defs::KEY_GET_LAUNCHERS)){
                launchers.clear();
                emit(SIGNAL(launchersClearred()));
                QString str = data.replace(Defs::KEY_GET_LAUNCHERS+", ", "");
                QList<QMap<QString, QString>> map = convertirJsonAListaDeMapas(str);
                for (auto &&it : map) {
                    QString pckg = it["LAUNCHER_PKG"];
                    QString label = it["LAUNCHER_LABEL"];
                    QString launch = it["LAUNCHER_ACTY"];
                    getIcon(Defs::KEY_FUP_ICONS_DIR+"/"+pckg);
                    LauncherInfo info(pckg, launch, label);
                    //signaler.installOn(btn);
                    launchers.append(info);
                    emit(SIGNAL(addLauncher(info)));
                }
                if(!launchers.isEmpty()){
                    emit(SIGNAL(launchersSet(launchers)));
                }
            }
        }

    }
}

void Device::getIcon(QString remotePath){
    QProcess p;
    QString localFolderIconPath = Defs::iconsPath();
    QDir().mkpath(localFolderIconPath);
    QFileInfo remoteInfo(remotePath);
    p.start(
        config->getAdbPath(),
        QString("-s "+ id() +" pull " + remotePath + " " + localFolderIconPath + "/" + remoteInfo.fileName())
            .split(" "));
    p.waitForFinished();
    QString out = p.readAll();
    //qDebug() << out;
}




void Device::parse(QStringList out){
    if(!out.isEmpty()){
        if(out.at(0) == Defs::KEY_GET_PACKAGES){
            out.removeAt(0);
            //updatePackages(out);
            appAdder->setList(out);
            appAdder->start();
        }
    }
}


void Device::stopScrcpy(){
    scrcpy.kill();
}

void Device::runScrcpy(QStringList params){
    QStringList args;
    args << "-s" << id() << params;
    scrcpy.start(config->getScrcpyPath(), args);
}


QList<QMap<QString, QString>> Device::convertirJsonAListaDeMapas(const QString &jsonString) {
    QList<QMap<QString, QString>> listaDeMapas;

    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (!doc.isArray()) {
        qDebug() << "JSON no es un array";
        qDebug() << jsonString;
        return listaDeMapas;
    }

    QJsonArray jsonArray = doc.array();
    foreach (const QJsonValue &jsonValue, jsonArray) {
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
