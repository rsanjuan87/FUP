

#include "AppAdder.h"

AppAdder::AppAdder(Config *config, QString deviceId, QString jsonFilePath, QObject* parent)
    : QThread(parent)
{
    this->config = config;
    this->id = deviceId;
    this->filePath = jsonFilePath;
}

void AppAdder::stop() { kill = true; }

void AppAdder::processStrJson(QString str){
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
}

void AppAdder::saveIcon(const QString pkgId, const QByteArray body){
    QString localFolderIconPath = Defs::localIconsPath(id);
    QDir().mkpath(localFolderIconPath);
    QFile file(localFolderIconPath + "/" + pkgId);
    //check if exists and same size in bits
    if(file.exists()){
        if(body.size() == file.size() && body.size() > 0){
            return;
        }else{
            file.remove();
        }
    }

    if (file.open(QIODevice::WriteOnly)) {
        file.write(body);
        file.close();
    } else {
        // Manejar error de apertura de archivo
    }
    for(LauncherInfo* i : launchers){
        if(i->pkgId == pkgId){
            emit addLauncher(i);
            break;
        }
    }
}

bool AppAdder::checkIcon(const QString pkgId, const int size){
    QString localFolderIconPath = Defs::localIconsPath(id);
    QDir().mkpath(localFolderIconPath);
    QFile file(localFolderIconPath + "/" + pkgId);
    //check if exists and same size in bits
    if(file.exists()){
        if(size == file.size() && size > 0){
            return true;
        }
    }
    return false;
}

QMap<QString, QString> AppAdder::toJson(const QString &jsonString)
{
    QMap<QString, QString> mapa;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());

    if (!jsonDoc.isObject())
        return mapa;

    QJsonObject jsonObject = jsonDoc.object();
    QJsonObject::iterator it;
    for (it = jsonObject.begin(); it != jsonObject.end(); ++it) {
        QString clave = it.key();
        QJsonValue valor = it.value();
        mapa.insert(clave, valor.toString());
    }

    return mapa;
}

QList<QMap<QString, QString> > AppAdder::convertirJsonAListaDeMapas(const QString &jsonString) {
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

void AppAdder::getIcon(QString remotePath){
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

void AppAdder::run(){
    //getFile(Defs::KEY_LAUNCHERS_FILE);
    QProcess p;
    // p.setTextModeEnabled(true);
    p.setProcessChannelMode(QProcess::MergedChannels);
    QStringList params;
    params << "-s" << id << "shell" << "run-as"<<  Defs::KEY_PACKAGE_ID <<"cat" << filePath;
    p.start(config->adbPath(), params);
    p.waitForFinished();
    QString str = p.readAll();
    // processStrJson(str);
    p.terminate();
    // this->deleteLater();
}
