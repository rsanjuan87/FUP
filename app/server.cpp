#include "server.h"

#include "devicesmanager.h"

#include <QTimer>


HttpMethod HttpServer::parseMethod(const QString &methodString)
{
    if (methodString == "GET") return HttpMethod::GET;
    if (methodString == "POST") return HttpMethod::POST;
    if (methodString == "PUT") return HttpMethod::PUT;
    if (methodString == "DELETE") return HttpMethod::DELETE;
    if (methodString == "PATCH") return HttpMethod::PATCH;
    if (methodString == "HEAD") return HttpMethod::HEAD;
    if (methodString == "OPTIONS") return HttpMethod::OPTIONS;
    return HttpMethod::UNKNOWN;
}

void HttpServer::setManager(DevicesManager *manager){
    this->devManager = manager;
}

void HttpServer::start(){
    if (!listen(QHostAddress::Any, Defs::serverPort))
    {
        qDebug() << "Could not start the server";
        throw "Could not start the server";
    }

    qDebug() << "Server listening on port "<< Defs::serverPort;
}

void HttpServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);

    connect(socket, &QTcpSocket::readyRead, this, &HttpServer::readClient);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

void HttpServer::readClient()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (socket == nullptr) return;

    static QHash<QTcpSocket*, QByteArray> partialData;

    QByteArray newData = socket->readAll();

    if (newData.isEmpty()) {
        return;
    }

    QByteArray &buffer = partialData[socket];
    buffer.append(newData);

    int headerEnd = buffer.indexOf("\r\n\r\n");
    if (headerEnd == -1) {
        QTimer::singleShot(0, this, [this]{ readClient(); });
        return;
    }

    QByteArray header = buffer.mid(0, headerEnd);
    buffer.remove(0, headerEnd + 4); // Eliminamos el encabezado y los \r\n\r\n

    QStringList requestLines = QString(header).split("\r\n");

    if (requestLines.isEmpty()) {
        sendResponse(socket, "400 Bad Request", "Invalid request");
        return;
    }

    QStringList requestParts = requestLines[0].split(" ");
    if (requestParts.size() < 2) {
        sendResponse(socket, "400 Bad Request", "Invalid request format");
        return;
    }

    QString method = requestParts[0];
    QString path = requestParts[1];

    QMap<QString, QString> headers;
    for (int i = 1; i < requestLines.size(); ++i) {
        QString line = requestLines[i];
        if (line.contains(": ")) {
            QStringList parts = line.split(": ");
            headers.insert(parts.first(), parts.last());
        }
    }

    HttpMethod httpMethod = parseMethod(method);
    if(httpMethod == HttpMethod::UNKNOWN){
        method = newData.split(' ')[0];
        httpMethod = parseMethod(method);
    }
    int contentLength = headers.value("Content-Length", "0").toInt();

    switch (httpMethod) {
    case HttpMethod::POST:{
        bool end = preHandlePost(socket, path, headers);
        if (end) {
            qDebug() << "ignoring";
            socket->disconnectFromHost();
            return;
        }
    }break;
    case HttpMethod::GET:
    case HttpMethod::OPTIONS:
    default:
        //do nothing yet
        break;
    }

    if (contentLength > 0 && buffer.size() >= contentLength) {
        QString body = QString(buffer.mid(0, contentLength));
        buffer.remove(0, contentLength);

        switch (httpMethod) {
        case HttpMethod::GET:
            handleGet(socket, path, headers);
            break;
        case HttpMethod::POST:
            handlePost(socket, path, headers, body);
            break;
        case HttpMethod::OPTIONS:
            sendResponse(socket, "200 OK", "Options are OK");
            break;
        // Agrega otros casos para métodos HTTP si es necesario
        default:
            sendResponse(socket, "405 Method Not Allowed", "Unsupported request method");
        }

        partialData.remove(socket);
    } else {
        QTimer::singleShot(0, this, [this]{ readClient(); });
    }

    socket->disconnectFromHost();
}


void HttpServer::handleGet(QTcpSocket *socket, const QString &path, const QMap<QString, QString> &headers)
{
    QString deviceId = headers.value("deviceId");
    // Simple routing
    if (path == "/") {
        sendResponse(socket, "200 OK", "Welcome to Qt HTTP Server!");
    } else if (path == "/about") {
        sendResponse(socket, "200 OK", "This is a simple Qt-based HTTP server.");
    } else {
        sendResponse(socket, "404 Not Found", "The requested resource was not found.");
    }
}


/**
 * @brief HttpServer::preHandlePost
 * @param path
 * @param headers
 * @return if may should ends processing request
 */
bool HttpServer::preHandlePost(QTcpSocket *socket, const QString &path, const QMap<QString, QString> &headers)
{
    QString deviceId = headers.value("deviceId");
    Device* dev = devManager->get(deviceId);
    if(dev == nullptr){
        qDebug() << "no device" << deviceId;
        return false;
    }

    switch (parseEndPoint(path)) {
    case EndPoints::launcher_icon:{
        qDebug() << "prereceived launcher icon" << deviceId;
        int originalSize = headers.value("originalSize", "0").toInt();
        QString pkgId = headers.value("pkgId");
        sendResponse(socket, "204 OK", "More data no needed");
        bool end = dev->checkIcon(pkgId, originalSize);
        return end;
    }break;
    case EndPoints::launchers:
    case EndPoints::notification:
    case EndPoints::notifStatus:
    default:
        //do nothing
        break;
    }
    return false;
}

/**
 * @brief HttpServer::handlePost
 * @param socket
 * @param path
 * @param headers
 * @param body
 */
void HttpServer::handlePost(QTcpSocket *socket, const QString &path, const QMap<QString, QString> &headers, const QString body)
{
    QString deviceId = headers.value("deviceId");
    Device* dev = devManager->get(deviceId);
    if(dev == nullptr){
        qDebug() << "no device" << deviceId;
        return;
    }

    switch (parseEndPoint(path)) {
    case EndPoints::launchers:{
        qDebug() << "received launchers" << deviceId;
        QString notifStatus = headers.value("notifStatus");
        dev->setAllowedNotif(notifStatus == "true");
        dev->processLaunchers(body);
        return sendResponse(socket, "200 OK", "received");
    }break;
    case EndPoints::launcher_icon:{
        qDebug() << "received launcher icon" << deviceId;
        QString pkgId = headers.value("pkgId");
        QByteArray base64 = QByteArray::fromBase64(body.toUtf8());
        dev->saveIcon(pkgId, base64);
        return sendResponse(socket, "200 OK", "received");
    }break;
    case EndPoints::notification:{
        qDebug() << "received notifications" << deviceId;
        // QString pkgId = headers.value("pkgId");
        dev->parseNotif(body, deviceId);
        return sendResponse(socket, "200 OK", "received");
    }break;
    case EndPoints::notifStatus:{
        qDebug() << "received notification status" << deviceId;
        dev->parseStatus(body);
        return sendResponse(socket, "200 OK", "received");
    }break;
    default:
        qDebug() << "received 404"<< path << deviceId;
        return sendResponse(socket, "404 Not found", "received");
        break;
    }
}

void HttpServer::sendResponse(QTcpSocket *socket, const QString &status, const QString &body)
{
    QByteArray response = "HTTP/1.1 " + status.toUtf8() + "\r\n";
    response += "Content-Type: text/plain\r\n";
    response += "Content-Length: " + QByteArray::number(body.length()) + "\r\n";
    response += "\r\n";
    response += body.toUtf8();

    socket->write(response);
}
EndPoints HttpServer::parseEndPoint(const QString &pathString) {
    if (pathString == "/notifStatus")
        return EndPoints::notifStatus;
    if (pathString == "/launchers")
        return EndPoints::launchers;
    if (pathString == "/launcher-icon")
        return EndPoints::launcher_icon;
    if (pathString == "/notification")
        return EndPoints::notification;
    return EndPoints::unknown;
}
