#include "devicesmanager.h"

DevicesManager::DevicesManager(Config *config, QSystemTrayIcon *tray, QObject *parent)
    : QObject(parent) {
    // QThread(parent) {
    this->tray = tray;
    this->config = config;
}

DevicesManager::~DevicesManager(){
    QStringList keys = devices.keys();
    foreach (QString key, keys) {
        Device *dev = devices.take(key);
        dev->disconnectDevice();
        delete dev;
    }
}

Device *DevicesManager::value(QString id){
    return devices.value(id);
}

void DevicesManager::diconectDevice(QString id){
    Device* dev = devices.value(id);
    dev->disconnectDevice();
    // dev->deleteLater();
    delete dev;
}

void DevicesManager::start(){
    p = new QProcess;
    p->setProcessChannelMode(QProcess::MergedChannels);
    QStringList params;
    params << "devices" << "-l";
    connect(p, SIGNAL(finished(int)), this, SLOT(readOut(int)));
    p->start(config->adbPath(), params);
}

void DevicesManager::readOut(int){
    QString out = QString(p->readAll()).remove("\r");
    // p->deleteLater();
    QStringList lines = out.split("\n");
    foreach (QString l, lines) {
        if (l.isEmpty() || l.contains("List of devices") || l.contains("* daemon"))
            continue;
        else{
            Device* dev = new Device(l, config, tray, parent());
            // connect(dev, &Device::addLauncher, this, &DevicesManager::addLauncherSlot);
            // connect(dev, &Device::launchersClearred, this, &DevicesManager::launchersClearredSlot);
            // connect(dev, &Device::launchersSet, this, &DevicesManager::launchersSetSlot);
            connect(dev, &Device::deviceDisconected, this, &DevicesManager::deviceDisconectedSlot);
            QString key = dev->id();
            try{
                if(devices.contains(key)){
                        Device *oldDev = devices.value(key);
                        if(oldDev->status() == dev->status()) {
                            continue;
                        } else {
                            oldDev->setLine(l);
                            delete dev;
                            emit deviceUpdated(dev);
                        }
                } else {
                    //force add on no exists
                    throw QException();
                }
            }catch(QException){
                devices.insert(key, dev);
                emit deviceAdded(dev);
            }
        }
    }
    QStringList keys = devices.keys();
    foreach (QString key , keys) {
        if (!out.contains(key)) {
            deviceRemovedSlot(key);
            emit deviceRemoved(key);
        }
    }
    emit devicesConnected(devices.values());
}

// void DevicesManager::run(){
    // start();
    // QProcess p;
    // p.setProcessChannelMode(QProcess::MergedChannels);
    // QStringList params;
    // params << "devices" << "-l";
    // p.start(config->adbPath(), params);
    // p.waitForFinished();
    // QString out = QString(p.readAll()).remove("\r");
    // QStringList lines = out.split("\n");


    // foreach (QString l, lines) {
    //     if (l.isEmpty() || l.contains("List of devices") || l.contains("* daemon"))
    //         continue;
    //     else{
    //         Device* dev = new Device(l, config, tray, parent());
    //         // connect(dev, &Device::addLauncher, this, &DevicesManager::addLauncherSlot);
    //         // connect(dev, &Device::launchersClearred, this, &DevicesManager::launchersClearredSlot);
    //         // connect(dev, &Device::launchersSet, this, &DevicesManager::launchersSetSlot);
    //         connect(dev, &Device::deviceDisconected, this, &DevicesManager::deviceDisconectedSlot);
    //         Device *oldDev = devices.value(dev->id());
    //         if (oldDev != nullptr){
    //             if(oldDev->status() == dev->status()) {
    //                 continue;
    //             } else {
    //                 devices.insert(dev->id(), dev);
    //                 emit deviceAdded(dev);
    //             }
    //         } else {
    //             devices.insert(dev->id(), dev);
    //             emit deviceAdded(dev);
    //         }
    //     }
    // }
    // QStringList keys = devices.keys();
    // foreach (QString key , keys) {
    //     if (!out.contains(key)) {
    //         Device *dev = devices.take(key);
    //         QString id = dev->id();
    //         dev->disconnectDevice();
    //         delete dev;
    //         emit deviceRemoved(id);
    //     }
    // }
    // emit devicesConnected(devices.values());
// }

bool DevicesManager::isEmpty() { return devices.empty(); }


// void DevicesManager::addLauncherSlot(LauncherInfo* info, QString id){
//     emit addLauncher(info, id);
// }
// void DevicesManager::launchersClearredSlot(QString id){
//     emit launchersClearred(id);
// }
// void DevicesManager::launchersSetSlot(QSet<LauncherInfo*> set, QString id){
//     emit launchersSet(set, id);
// }

void DevicesManager::deviceDisconectedSlot(QString id){
    emit deviceDisconected(id);
}
void DevicesManager::deviceRemovedSlot(QString id) {
    if(!devices.contains(id))
        return;
    Device *dev = devices.take(id);
    if (!dev->nulled()) {
        dev->disconnectDevice();
        delete dev;
    }
}
