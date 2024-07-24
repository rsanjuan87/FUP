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

    //true -> open the apps on new virtual display
    bool coherenceMode = true;

private:

    QString adbPath = "/Users/san/Library/Android/sdk/platform-tools/adb";
    QString scrcpyPath = "/opt/homebrew/bin/scrcpy";

signals:

};

#endif // CONFIG_H
