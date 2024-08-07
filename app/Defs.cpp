#include "Defs.h"


QString Defs::KEY_NOTIFICACTION_CHANNEL = "NOTIFICATION_CHANNEL";
QString Defs::KEY_PACKAGE_ID = "org.santech.fup";

QString Defs::KEY_GET_PACKAGES = "fup:packages";

QString Defs::KEY_GET_PACKAGE_INFO = "fup:packages_info";
QString Defs::KEY_GET_PACKAGE_ICON = "fup:packages_icon";

QString Defs::KEY_GET_LAUNCHERS ="fup:get_launchers";
QString Defs::KEY_GET_LABEL ="fup:label";
QString Defs::KEY_LAUNCHERS_FILE ="launchers.json";

QString Defs::KEY_SAVED_PACKAGE_ICON = "fup:icon_saved";

QString Defs::KEY_RUN_PACKAGE = "fup:run_package";

QString Defs::KEY_START_SERVICE = "fup:start_service";
QString Defs::KEY_STOP_SERVICE = "fup:stop_service";

QString Defs::KEY_APP_RESUMED = "fup:app_resumed";

QString Defs::KEY_LAUCH_ACTIVITY = "fup:launch_activity";

QString Defs::KEY_GET_NOTIFICATIONS_STATUS = "fup:get_notifications_status";
QString Defs::KEY_GET_REQUEST_NOTIFICATIONS_ACCESS = "fup:request_notifications_access";


QString Defs::getPackagesKey(){return Defs::actionHeader(Defs::KEY_GET_PACKAGES);};

QString Defs::actionHeader(QString action){return "echo \""+action+"\" &&";}

QString Defs::localIconsPath(const QString id){
    return localPath(id)+"/icons/";
}

QString Defs::localPath(const QString id){
    QString _id = QString(id).replace(":", "_");
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation)+"/"+Defs::KEY_PACKAGE_ID+"/"+_id+"/";
}
