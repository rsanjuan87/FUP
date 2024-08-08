#ifndef CONFIG_H
#define CONFIG_H

#include <QMap>
#include <QObject>

class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(QObject *parent = nullptr);
    QString adbPath(bool forRun = true);
    QString scrcpyPath(bool forRun = true);
    QString scrcpyServerPath(bool forRun = true);

    static QString adbDefaultPath;
    static QString scrcpyDefaultPath;
    static QString scrcpyServerDefaultPath;

    //true -> open the apps on new virtual display
    bool coherenceMode = true;

    void setAdbPath(QString v);
    void setScrcpyPath(QString v);
    void setScrcpyServerPath(QString v);
public slots:
    void load();
    void save();
private:

#ifdef Q_OS_WIN
    QString _adbPath = "C:/platform-tools/adb.exe";
#else
    QString _adbPath = "/Users/san/Library/Android/sdk/platform-tools/adb";
#endif

    // QString scrcpyPath = "/opt/homebrew/bin/scrcpy";
    QString _scrcpyPath = "/Users/san/Projects/FUP/app/scrcpy/builddir/app/scrcpy";
    QString _scrpcyServerPath = "/Users/san/Projects/FUP/app/scrcpy/builddir/server/scrcpy-server";

signals:
    void loaded();
    void saved();

};

#endif // CONFIG_H
