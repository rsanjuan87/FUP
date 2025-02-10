#ifndef SERVER_H
#define SERVER_H

#include "Defs.h"
#include "devicesmanager.h"
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    HEAD,
    OPTIONS,
    UNKNOWN


};


enum class EndPoints{
    notifStatus,
    launchers,
    launcher_icon,
    notification,
    unknown
};


class HttpServer : public QTcpServer
{
    Q_OBJECT

public:
    HttpServer(QObject *parent = nullptr) : QTcpServer(parent) {}
    HttpMethod parseMethod(const QString &methodString);
    EndPoints parseEndPoint(const QString &pathString);

    void setManager(DevicesManager *manager);
public slots:
    void start();

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void readClient();
    void handleGet(QTcpSocket *socket, const QString &path, const QMap<QString, QString> &headers);
    bool preHandlePost(QTcpSocket *socket, const QString &path, const QMap<QString, QString> &headers);
    void handlePost(QTcpSocket *socket, const QString &path, const QMap<QString, QString> &headers, const QString body);
    void sendResponse(QTcpSocket *socket, const QString &status, const QString &body);

private:
    DevicesManager *devManager;
};



#endif // SERVER_H
