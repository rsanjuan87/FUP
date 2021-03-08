#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>

class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(QObject *parent = nullptr);
    inline QString getAdbPath() {return adbPath;}
    inline QString getScrcpyPath() {return scrcpyPath;}


private:

    QString adbPath = "/Volumes/Datos/Databank/programacion/sdk/platform-tools/adb";
    QString scrcpyPath = "/usr/local/Cellar/scrcpy/1.16_2/bin/scrcpy";

signals:

};

#endif // CONFIG_H
