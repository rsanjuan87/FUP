
#include "Defs.h"
#include "Defs.h"
#include "Device.h"

#include <QJsonDocument>
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsonobject.h>
#include <QMap>
#include <QProcess>
#include <QSet>
#include <QGuiApplication>
#include <QScreen>
#include <QSvgRenderer>
#include <QPainter>
#include <QMessageBox>

#ifdef Q_OS_WIN
    #include <windows.h>
#endif

Device::Device(QString line, Config *conf, QSystemTrayIcon *t, QObject *parent)
    : QObject{parent},
     notifHelper(new NotificationHelper(t))
{

    while (line.contains("  ")) {
        line = line.replace("  ", " ");
    }
    line= line.trimmed();
    this->_line = line;
    this->config = conf;
    init();
}

void Device::init(){
    // adb.setProcessChannelMode(QProcess::MergedChannels);
    // connect(&adb, SIGNAL(finished(int)), this, SLOT(readAdb(int)));


    logcat.setProcessChannelMode(QProcess::MergedChannels);
    connect(&logcat, &QIODevice::readyRead, this, &Device::readLogcat);
    connect(&logcat, &QProcess::readyReadStandardError, this, &Device::readLogcat);
    connect(&logcat, &QProcess::readyReadStandardOutput, this, &Device::readLogcat);
    connect(&logcat, &QProcess::finished, this, [this]{
        emit deviceDisconected(id());
    });

    appAdder = new AppAdder(config, id(), remoteFupDir()+"/launchers.json", parent());
    connect(appAdder, &AppAdder::addLauncher, this, &Device::addedLauncherSlot);
    connect(appAdder, &AppAdder::launchersClearred, this, &Device::Cleared);
    connect(appAdder, &AppAdder::launchersSet, this, &Device::launchersSetSlot);

    connect(appAdder, &AppAdder::deviceDiconected, this, &Device::deviceDisconectedSlot);

    appAdder->start();
    screenSize();
    screenDpi();
    lastScreenId();
}



Device::~Device(){
    // adb.terminate();
    const QStringList keys = scrcpyProcess.keys();
    for(const QString &key: keys){
        QProcess * p = scrcpyProcess.take(key);
        if(p != nullptr){
            p->terminate();
            // p->deleteLater();
            delete p;
        }
    }
    logcat.terminate();
}

QString Device::id(){
    return _line.split(" ").first();
}

QString Device::status(){
    try{
        if (nulled()) {
            throw QException();
        }
        if(_line.contains("product")){
            return _line.split(" product:").first().split(" ")[1];
        }else{
            return _line.split(" transport_id:").first().split(" ").last();
        }
    }catch(QException e){
        qDebug() << e.what();
    }
    return "offline";
}

QString Device::product(){
    return _line.split(" product:").last().split(" model:").first();
}

QString Device::model(){
    if(_line.contains("model:")){
        return _line.split(" model:").last().split(" device:").first();
    }else{
        return id();
    }
}

QString Device::device(){
    return _line.split(" device:").last().split(" transport_id:").first();
}

QString Device::transportId(){
    return _line.split(" transport_id:").last();
}


QString Device::screenSize(QString screenId){
    // if(_screenSize.isEmpty()){
    QString _screenSize;
        QProcess p;
        p.setProcessChannelMode(QProcess::MergedChannels);
        QStringList params;
        params << "-s" << id() << "shell" << "wm" << "size" << "-d" << screenId;
        p.start(config->adbPath(), params);
        p.waitForFinished();
        QString out = QString(p.readAll()).remove("\r");
        QString line = out.trimmed();
        _screenSize = QString(line.split(": ").last().remove("\n"));
    // }
    return _screenSize;
}


QString Device::screenDpi(QString screenId){
    // if(_dpi.isEmpty()){
        QString _dpi;
        QProcess p;
        p.setProcessChannelMode(QProcess::MergedChannels);
        QStringList params;
        params << "-s" << id() << "shell" << "wm" << "density" << "-d" << screenId;
        p.start(config->adbPath(), params);
        p.waitForFinished();
        QString out = QString(p.readAll()).remove("\r");
        QStringList lines = out.trimmed().split("\n");
        for(QString line: lines){
            if(line.contains("Physical")){
                _dpi = QString(line.split(": ").last().remove("\n"));
                break;
            }
        }
        if(_dpi.isEmpty()){
            _dpi = QString(out.split(": ").last().remove("\n"));
        }
    // }
    return _dpi;
}


QStringList Device::screens(){
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    QStringList params;
    params << "-s" << id() << "shell" << "dumpsys" << "display" << "|" << "grep" << "mDisplayId=";
    p.start(config->adbPath(), params);
    p.waitForFinished();
    QString out = QString(p.readAll()).remove("\r");
    if(out.contains("not found")){
        emit deviceDisconected(id());
        // throw DeviceDisconectedException();
    }
    QSet<int> set;
    for(QString l : out.split("\n")){
        if(l.isEmpty())continue;
        set <<  l.remove("mDisplayId=").remove(" ").remove(":").toInt();
    }
    QList<int> list = set.values();
    std::sort(list.begin(), list.end());
    QStringList res;
    for(int e : list){
        res << QString().setNum(e);
    }

    return res;
}

QSet<LauncherInfo *> Device::launchers()
{
    if (this != nullptr && appAdder != nullptr) {
        return appAdder->launchers;
    } else {
        // Manejo cuando appAdder es nulo
        return QSet<LauncherInfo*>();
    }

}

QString Device::remoteFupDir(){
    if (_fupDir.isEmpty()){
        QProcess p;
        p.setProcessChannelMode(QProcess::MergedChannels);
        QStringList params;
        params << "-s" << id() << "shell" << "run-as org.santech.fup pwd";
        p.start(config->adbPath(), params);
        p.waitForFinished();
        QString out = QString(p.readAll()).remove("\r");
        out = out.trimmed()+"/files";
        _fupDir = out;
    }
    return _fupDir;
}

QString Device::remoteFupIconsDir(){
    return QString(remoteFupDir() + "/icons");
}

void Device::connectDevice(){

    if (this == nullptr || logcat.state() == QProcess::Running) {
        return;
    }

    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);

    // /////creating remote folders
    // QStringList params;
    // params << "-s" << id() << "shell mkdir -p " + Defs::remoteFupIconsDir() + " && chmod 777 -R " + Defs::remoteFupDir();
    // p.start(config->adbPath(), cmd.split(" "));
    // p.waitForFinished();
    // QString out = QString(p.readAll()).remove("\r");

    // //clean p readstream
    // QString(p.readAll()).remove("\r");

    //get remote date
    p.start(config->adbPath() , QString("-s "+ id() +" shell date +'%m-%d %H:%M:%S.000'").split(" "));
    p.waitForFinished();
    QString time = QString(p.readAll()).remove("\r").replace("\n", "");
    //to connnect logcat with that date
    QStringList params;
    // QString("-s "+ id() +" shell logcat -s "+ Defs::KEY_PACKAGE_ID + " -T '"+time+"'").split(" ")
    params << "-s" << id() << "logcat" << "-s" << Defs::KEY_PACKAGE_ID << "-T" << time;
    logcat.start(config->adbPath(), params);

    // runInAdb("monkey -p " + Defs::KEY_PACKAGE_ID + "  -c android.intent.category.LAUNCHER 1");

    requestAction(Defs::KEY_START_SERVICE);
    requestAction(Defs::KEY_GET_LAUNCHERS);
}

void Device::disconnectDevice(){
    if(nulled())
        return;
    logcat.terminate();
    disconnect(&logcat, &QIODevice::readyRead, this, &Device::readLogcat);
    disconnect(&logcat, &QProcess::readyReadStandardError, this,
               &Device::readLogcat);
    disconnect(&logcat, &QProcess::readyReadStandardOutput, this,
               &Device::readLogcat);
}

void Device::requestAction(QString action, std::function<void(QProcess*)> onAdbFinished){
    return requestAction(action, QMap<QString, QString>(), onAdbFinished);
}

void Device::requestAction(QString action, QMap<QString, QString> extras, std::function<void(QProcess*)> onAdbFinished) {
    QProcess* p = new QProcess(0);
    p->setProcessChannelMode(QProcess::MergedChannels);
    QStringList params = QStringList()
                         << "-s" << id() <<"shell" << "am" << "broadcast" << "-a"
                         << "android.intent.action.VIEW" << "-n"
                         << Defs::KEY_PACKAGE_ID + "/" +
                                Defs::KEY_PACKAGE_ID + ".BroadcastReceiver"
                         << "--es" << "action" << "'" + action + "'";
    for (auto it = extras.constBegin(); it != extras.constEnd(); ++it) {
        params << "--es" << it.key() << it.value();
    }
    QObject::connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, p, onAdbFinished](int, QProcess::ExitStatus){
        QMetaObject::invokeMethod(this, [p, onAdbFinished]() {
            if(onAdbFinished != nullptr){
                onAdbFinished(p);
            }
            // p->deleteLater();
            delete p;
        }, Qt::QueuedConnection);
    });
    p->start(config->adbPath(), params);

}



void Device::getAppList(){
    runInAdb("pm list packages -3");
}

// void Device::readAdb(int){
//     QStringList out = QString(adb.readAll().remove("\r")).split('\n');
//     parse(out);
// }

void Device::readLogcat(){
    QStringList out = QString(QString(logcat.readAll()).remove("\r")).split('\n');
    parseMessages(out);
}

void Device::runInAdb(QString shell, std::function<void(QProcess*)> onAdbFinished){
    QStringList params;
    params << "-s" << id() << "shell" << shell;
    QProcess* p = new QProcess(0);
    p->setProcessChannelMode(QProcess::MergedChannels);

    QObject::connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, p, onAdbFinished](int, QProcess::ExitStatus){
        QMetaObject::invokeMethod(this, [p, onAdbFinished]() {
            if(onAdbFinished != nullptr){
                onAdbFinished(p);
            }
            // p->deleteLater();
            delete p;
        }, Qt::QueuedConnection);
    });

    p->start(config->adbPath(), params);
    // adbList << p;
}





// void Device::runInAdb(QString shell, std::function<void(QProcess*)> onAdbFinished){
//     QStringList params;
//     params << "-s" << id() << "shell" << shell;
//     QProcess* p = new QProcess(0);
//     p->setProcessChannelMode(QProcess::MergedChannels);
//     // connect(p, SIGNAL(finished(int)), this,  SLOT([p &onAdbFinished](int exitCode) {
//         // onAdbFinished(p);
//     // }));
//     connect(p, SIGNAL(finished(int)), this, SLOT([this p &onAdbFinished](int exitCode){
//                 QMetaObject::invokeMethod(this, [p]() {
//                     onAdbFinished(p);
//                 }, Qt::QueuedConnection);
//             }));
//     p->startDetached(config->adbPath(), params);
//     adbList << p;
// }


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
                notifHelper->parse(data, id());
            } else if (data.startsWith(Defs::KEY_GET_PACKAGE_ICON)) {
                /*QProcess p;
                QString path = data.remove(Defs::KEY_GET_PACKAGE_ICON + ", ");
                p.start(config->adbPath(), QStringList()
                                                  << "-s" << id() << "shell"
                                                            << "cat" << path << " || "
                                                            << "echo"
                                                            << "''"
                                                            << ">" << path);
                p.waitForFinished();
                p.readAll();*/
            } else if (data.startsWith(Defs::KEY_SAVED_PACKAGE_ICON)) {
                // QString remotePath = data.remove(Defs::KEY_SAVED_PACKAGE_ICON + ", ");
                // getIcon(remotePath);
            }else if (data.startsWith(Defs::KEY_GET_LAUNCHERS)){
                // //getFile(Defs::KEY_LAUNCHERS_FILE);
                // QProcess p;
                // p.setTextModeEnabled(true);
                // p.setProcessChannelMode(QProcess::MergedChannels);
                // QStringList params;
                QString filePath = data.remove(Defs::KEY_GET_LAUNCHERS + ", ");
                loadApps(filePath);
            }else if (data.startsWith(Defs::KEY_APP_RESUMED)){
                QString pkgId = data.remove(Defs::KEY_APP_RESUMED + ", ");
                if (config->coherenceMode){
                    QProcess* p = scrcpyProcess.take(pkgId);
                    if(p != nullptr){
                        p->terminate();
                    }
                }
            }

        }

    }
}

void Device::loadApps(QString path){
    if(this != nullptr && appAdder != nullptr && appAdder->isRunning()){
        return;
    }
    appAdder->filePath = path;
    appAdder->start();
}


void Device::deviceDisconectedSlot(){
    emit deviceDisconected(id());
}


void Device::addedLauncherSlot(LauncherInfo* info){
    emit addLauncher(info, id());
}


void Device::launchersSetSlot(QSet<LauncherInfo*> list){
    emit launchersSet(list, id());
}

void Device::Cleared(){
    emit launchersClearred(id());
}

// void Device::getIcon(QString remotePath){
//     QProcess p;
//     p.setProcessChannelMode(QProcess::MergedChannels);
//     QString localFolderIconPath = Defs::localIconsPath(id());
//     QDir().mkpath(localFolderIconPath);
//     QFileInfo remoteInfo(remotePath);
//     QStringList params;
//     params << "-s" << id() << "shell" << "run-as "+ Defs::KEY_PACKAGE_ID+" cat " + remotePath;
//     p.start(config->adbPath(),params);
//     p.waitForFinished();
//     QByteArray  out = QString(p.readAll()).remove("\r");

//     QFile file(localFolderIconPath + "/" + remoteInfo.fileName());
//     if (file.open(QIODevice::WriteOnly)) {
//         file.write(out);
//         file.close();
//     } else {
//         // Manejar error de apertura de archivo
//     }

// }




// void Device::parse(QStringList out){
//    /* if(!out.isEmpty()){
//         if(out.at(0) == Defs::KEY_GET_PACKAGES){
//             out.removeAt(0);
//             //updatePackages(out);
//             appAdder->setList(out);
//             appAdder->start();
//         }
//     }*/
// }

QProcess::ProcessState Device::scrcpyStatus(QString pkgId){
    if (scrcpyProcess.contains(pkgId)){
        return scrcpyProcess.value(pkgId)->state();
    }
    return QProcess::NotRunning;
}


void Device::stopScrcpy(QString pkgId){
    if(scrcpyProcess.contains(pkgId)){
        QProcess *p = scrcpyProcess.take(pkgId);
        if (p != nullptr){
            p->kill();
            // p->deleteLater();
            // delete p;
        }
    }
}

QPixmap svgToPix(QString path){
    QSvgRenderer renderer(*new QString(path));
    QPixmap pixmap(renderer.defaultSize());
    QPainter* painter =new QPainter(&pixmap);
    renderer.render(painter);
    painter->end();
    return pixmap;
}

void Device::raiseWindow(QString pkgid, QString title){
    return raiseWindow(title, scrcpyProcess.value(pkgid)->processId());
}

void Device::raiseWindow(QString title, int pid){
#ifdef Q_OS_WIN
    // Obtener la ventana de la aplicación hija
    HWND hwndHijo = FindWindow(NULL, title);

    if (hwndHijo != NULL)
    {
        // Permitir que la ventana hija tome el foco
        if (!AllowSetForegroundWindow(hwndHijo))
        {
            // Manejo de error si AllowSetForegroundWindow falla
        }

        DWORD dwResult = 0;
        BOOL bSuccess = SendMessageTimeout(hwndHijo, WM_USER + 1, 0, 0, SMTO_NORMAL, 1000, &dwResult);
        bool ok = bSuccess && dwResult == 0;
        // Enviar el mensaje personalizado para activar la ventana
        if (!ok)
        {
            // Manejo de error si enviarMensajeAPrimeroPlano falla
        }
    }
#elif defined Q_OS_MAC
    QString scriptAppleScript = R"(
    tell application "System Events"
        if exists (processes where name is ")" + title + R"(") then
            tell process ")" + title + R"("
                set frontmost to true
            end tell
        end if
    end tell
)";

    QProcess procesoAppleScript;
    procesoAppleScript.startDetached("osascript", {"-e", scriptAppleScript});


    // QString scriptAppleScript = R"(
    //     tell application ")" + title + R"("
    //         activate
    //     end tell
    // )";

// QProcess procesoAppleScript;
// procesoAppleScript.startDetached("osascript", {"-e", scriptAppleScript});
#elif defined Q_OS_LINUX
    // Utilizar xdotool para activar la ventana
    QProcess activateWindowProcess;
    activateWindowProcess.startDetached("xdotool", QStringList()
                                               << "windowactivate"
                                               << "--sync"
                                               << QString().number(pid));
#else
    // ???
#endif
    // qDebug()<< "window " + title + " raise send pid:" + QString().setNum(pid);
}

QString Device::screenId(QString pkgId){
    return scrcpyProcess.value(pkgId)->property("screenId").toString();
}

void Device::runScrcpy(QString pkgId, QString title, QStringList params, QString screenSize){
    if(nulled()){
        return;
    }
    if(pkgId == Defs::ActionVirtualDesktop){
        QPixmap pixmap(":/computer");
        if (pixmap.save(Defs::localIconsPath(id())+"/"+pkgId, "png"))
            qDebug() << Defs::localIconsPath(id())+"/"+pkgId;
    }else if(pkgId == Defs::ActionMainScreen){
        QPixmap pixmap(":/phone");
        if (pixmap.save(Defs::localIconsPath(id())+"/"+pkgId, "png"))
            qDebug() << Defs::localIconsPath(id())+"/"+pkgId;
    }

    if(scrcpyProcess.contains(pkgId)){
        QProcess* scrcpy = scrcpyProcess.value(pkgId, nullptr);
        if(scrcpy != nullptr){
            if (scrcpy->state() == QProcess::NotRunning) {
                stopScrcpy(pkgId);
            }else{
                raiseWindow(title, scrcpy->processId());
                return;
            }
        }
    }

    if(title == ""){
        title = model();
    }
    if(pkgId == Defs::ActionVirtualDesktop){
        QSize s = QGuiApplication::primaryScreen()->size();
        screenSize =
            QString().setNum(s.width()) + "x" + QString().setNum(s.height());
    }
    if(screenSize ==""){
        screenSize = this->screenSize();
    }
    QStringList args;
    args << "-s" << id() << "--no-cleanup";
    QProcess mklink;
    QFileInfo appLink(Defs::localPath(id())+"/apps/"+title);
    if(!appLink.dir().exists()){
        QDir().mkdir(appLink.dir().path());
    }
    mklink.setProcessChannelMode(QProcess::MergedChannels);
#ifdef Q_OS_WIN
    mklink.start("mklink", QStringList() << appLink.filePath() << config->scrcpyPath());
#else
    mklink.start("ln", QStringList() << "-s" << config->scrcpyPath() << appLink.filePath());
#endif
    mklink.waitForFinished();
    // QString out = mklink.readAll();

    QProcess* scrcpy = new QProcess(0);
    QProcessEnvironment env = scrcpy->processEnvironment();
    env.insert("PATH", env.value("PATH")+":"+QFileInfo(config->adbPath()).dir().path());
    env.insert("SCRCPY_ICON_PATH", Defs::localIconsPath(id())+"/"+pkgId);
    if(config->coherenceMode){
        if(pkgId != Defs::ActionMainScreen && pkgId != Defs::ActionAudio){
            args << "--create-new-display="+screenSize;
        }
    }
    env.insert("SCRCPY_SERVER_PATH", config->scrcpyServerPath());
    scrcpy->setProcessEnvironment(env);
    scrcpy->setProcessChannelMode(QProcess::MergedChannels);
    QString screenId = QString().setNum(lastScreenId().toInt()+1);
    scrcpy->setProperty("screenId", screenId);
    scrcpyProcess.insert(pkgId, scrcpy);
    connect(scrcpy, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, scrcpy, pkgId](int, QProcess::ExitStatus){
        QMetaObject::invokeMethod(this, [scrcpy, pkgId, this]() {
            scrcpyProcess.take(pkgId);
            QString out = QString(scrcpy->readAll()).remove("\r");
            qDebug() << scrcpy->exitCode()<< scrcpy->exitStatus() << out;
            // p->deleteLater();
            delete scrcpy;
            emit appClosed(pkgId);
        }, Qt::QueuedConnection);
    });

    connect(scrcpy, &QProcess::readyRead, this, [this, pkgId]() {
        QMetaObject::invokeMethod(
            this,
            [pkgId, this]() {
                if (pkgId != Defs::ActionMainScreen) {
                    QString screenId = "N/A";
                    QProcess* scr = scrcpyProcess.value(pkgId);
                    if(scr != nullptr){
                        QString out = scr->readAll();
                        // qDebug() << out;
                        if(out.contains("Virtual display id is ")){
                            screenId = out.split("Virtual display id is ").last().split("\n").first().remove("\r");
                            _lastScreenId = screenId;
                        }
                        QString dpi = QString().setNum(screenDpi().toDouble() / 2);
                        if (screenId != "0" && screenId != "N/A")
                            runInAdb("wm density " + dpi + " -d " + screenId);
                    }
                }
            },
            Qt::QueuedConnection);
    });


    if(pkgId != Defs::ActionAudio){
        args << "--no-audio";
    }
    args << params;
    args << "--window-title="+title;
    scrcpy->setProcessEnvironment(env);
    scrcpy->setWorkingDirectory(QFileInfo(config->scrcpyPath()).dir().absolutePath());
    scrcpy->start(appLink.absoluteFilePath(), args);
}

void Device::setLine(QString line) {
    _line = line;
    emit deviceUpdated(id());
}

QString Device::lastScreenId() {
    if (_lastScreenId.isEmpty()) {
        QProcess p;
        p.setProcessChannelMode(QProcess::MergedChannels);
        QProcessEnvironment env = p.processEnvironment();
        env.insert("PATH", env.value("PATH") + ":" +
                               QFileInfo(config->adbPath()).dir().path());
        env.insert("SCRCPY_SERVER_PATH", config->scrcpyServerPath());

        QPixmap pixmap(":/refresh");
        QString path = Defs::localIconsPath(id())+"/refresh";
        pixmap.save(path, "png");
        env.insert("SCRCPY_ICON_PATH", path);
        p.setProcessEnvironment(env);

        p.start(config->scrcpyPath(), {"--create-new-display=10x10"});
        while (p.waitForReadyRead()) {
            // Virtual display id is
            QString out = p.readAll();
            if(out.contains("Virtual display id is ")){
                _lastScreenId = out.split("Virtual display id is ")
                .last()
                    .split("\n")
                    .first()
                    .remove("\r");
                p.terminate();
                p.kill();
                break;
            }else{
                continue;
            }
            if(p.state()!= QProcess::Running){
                break;
            }
        }
    }
    return _lastScreenId;
}
