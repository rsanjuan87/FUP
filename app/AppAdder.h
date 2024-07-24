#ifndef APPADDER_H
#define APPADDER_H


#include <QtCore/qobjectdefs.h>
#include <QProcess>
#include <QThread>

#include "config.h"

class AppAdder : public QThread{
    Q_OBJECT
    Config *config;
    QStringList out;
public:
    AppAdder(Config *config){
        this->config = config;
    }
    void setList(QStringList out){
        this->out = out;
    }

signals:
    void addApp(QString id);

protected:
    void run() override{
        updatePackages(out);
        this->deleteLater();
    };


    void updatePackages(QStringList list){
        //starts with package:
        for(int i= 0; i < list.size(); i++){
            QString s = list.at(i);
            QString id = s.replace("package:", "");
            if(s.isEmpty()) continue;

            QProcess a;
            a.start(config->getAdbPath(), QStringList(" shell \"cmd package resolve-activity --brief "+id+" \""));
            a.waitForFinished();
            QString out = QString(a.readAllStandardOutput());
            if(out.contains("No activity found")) continue;

            emit addApp(id);
        }
    }
};
#endif // APPADDER_H
