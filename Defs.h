#ifndef DEFS_H
#define DEFS_H

#include <QStandardPaths>
#include <QString>


class Defs{
public:
    inline static const QString KEY_NOTIFICACTION_CHANNEL = "NOTIFICATION_CHANNEL";
    inline static const QString KEY_PACKAGE_ID = "org.santech.notifyme";
    inline static const QString KEY_GET_PACKAGES = "fup:packages";
    inline static const QString KEY_GET_PACKAGE_INFO = "fup:packages_info";
    inline static const QString KEY_GET_PACKAGE_ICON = "fup:packages_icon";
    inline static const QString KEY_GET_LAUNCHERS ="fup:get_launchers";
    inline static const QString KEY_SAVED_PACKAGE_ICON = "fup:icon_saved";
    inline static const QString KEY_RUN_PACKAGE = "fup:run_package";
    inline static const QString KEY_START_SERVICE = "fup:start_service";
    inline static const QString KEY_STOP_SERVICE = "fup:stop_service";

    inline static QString getPackagesKey(){return actionHeader(KEY_GET_PACKAGES);};

    inline static QString actionHeader(QString action){return "echo \""+action+"\" &&";}

    inline static QString iconsPath(){
        return QStandardPaths::writableLocation(QStandardPaths::TempLocation)+"/icons/";
    }
};

#endif // DEFS_H
