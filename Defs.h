#ifndef DEFS_H
#define DEFS_H

#include <QString>


class Defs{
public:
    inline static const QString KEY_GET_PACKAGES = "fup:packages";
    inline static const QString KEY_GET_PACKAGE_INFO = "fup:packages_info";
    inline static const QString KEY_RUN_PACKAGE = "fup:run_package";

    inline static QString getPackagesKey(){return actionHeader(KEY_GET_PACKAGES);};

    inline static QString actionHeader(QString action){return "echo \""+action+"\" &&";}
};

#endif // DEFS_H
