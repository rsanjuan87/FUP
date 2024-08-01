#ifndef DEVICELISTLOADER_H
#define DEVICELISTLOADER_H

#include "config.h"

#include <QProcess>
#include <QThread>



class DeviceListLoader : public QThread{
    Q_OBJECT
    Config *config;
public:
    DeviceListLoader(Config *config){
        this->config = config;
    }

signals:
    void devices(QStringList);

protected:
    void run() override{
        QProcess p;
        p.setProcessChannelMode(QProcess::MergedChannels);
        p.start(config->adbPath(), QString("devices -l").split(" "));
        p.waitForFinished();
        QString out = p.readAll();
        QStringList lines = out.split("\n");
        emit devices(lines);
    };

};
#endif // DEVICELISTLOADER_H
