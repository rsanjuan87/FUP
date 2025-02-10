#include "launcherinfo.h"

LauncherInfo::LauncherInfo(QString pkgId, QString activityId, QString label, QString icon):
    pkgId(pkgId),
    activityId(activityId),
    label(label),
    icon(icon)
{}

LauncherInfo::LauncherInfo(const LauncherInfo &info)
{
    pkgId = info.pkgId;
    activityId = info.activityId;
    label = info.label;
    icon = info.icon;
}


