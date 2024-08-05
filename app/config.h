#ifndef CONFIG_H
#define CONFIG_H

#include <QMap>
#include <QObject>

class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(QObject *parent = nullptr);
    inline QString adbPath() {return _adbPath;}
    inline QString scrcpyPath() {return _scrcpyPath;}
    inline QString scrcpyServePath(){return _scrpcpyServerPath;}

    //true -> open the apps on new virtual display
    bool coherenceMode = true;

private:

#ifdef Q_OS_WIN
    QString _adbPath = "C:/platform-tools/adb.exe";
#else
    QString _adbPath = "/Users/san/Library/Android/sdk/platform-tools/adb";
#endif

    // QString scrcpyPath = "/opt/homebrew/bin/scrcpy";
    QString _scrcpyPath = "/Users/san/Projects/contrib/scc/scrcpy/builddir/app/scrcpy";
    QString _scrpcpyServerPath = "/Users/san/Projects/contrib/scc/scrcpy/builddir/server/scrcpy-server";

signals:

};

#endif // CONFIG_H
