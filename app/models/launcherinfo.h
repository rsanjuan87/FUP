#ifndef LAUNCHERINFO_H
#define LAUNCHERINFO_H

#include <QObject>

class LauncherInfo
{
public:
    explicit LauncherInfo(QString pkgId, QString actyId, QString label, QString icon = "");
    explicit LauncherInfo(LauncherInfo const &info);

    QString pkgId;
    QString activityId;
    QString label;
    QString icon;
};

Q_DECLARE_METATYPE(LauncherInfo*)

#endif // LAUNCHERINFO_H
