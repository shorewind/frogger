#include "client.h"
#include "graphicsdialog.h"
#include "dialog.h"
#include "defs.h"

Client::Client(Dialog *parent) :
    QDialog(parent)
{
    mainDialog = parent;

    setLocalIpAddress();
    mainDialog->portEdit->setText(QString::number(DEFAULT_PORT));  // default port set manually
}


void Client::connectToServer()
{
    socket = new QUdpSocket(this);

    ip = mainDialog->ipEdit->text();
    port = mainDialog->portEdit->text().toUInt();

    socket->connectToHost(QHostAddress(ip), port);
    connect(socket, &QUdpSocket::readyRead, this, &Client::processMsg);

    mainDialog->ipEdit->clear();
    mainDialog->portEdit->clear();

    if (socket->isValid())
    {
        mainDialog->textBrowser->append("Connected to Server: " + ip + ":" + QString::number(port));
        mainDialog->sendButton->setEnabled(true);
        mainDialog->connectButton->setEnabled(false);

        // Send initial JSON "CONNECT" message
        QJsonObject connectMessage;
        connectMessage["type"] = "CONNECT";

        sendJson(connectMessage);

        connect(mainDialog->sendButton, &QPushButton::clicked, this, &Client::sendMsg);

        graphicsDialog = new GraphicsDialog(this);
        graphicsDialog->show();
        connect(graphicsDialog, &GraphicsDialog::requestClose, this, &Client::disconnectFromServer);
    }
}

void Client::closeEvent(QCloseEvent *event)
{
    event->accept();
    disconnectFromServer();
}

void Client::disconnectFromServer()
{
    qDebug() << "disconnect";
    QJsonObject disconnectMessage;

    graphicsDialog->removePlayer(activeClientId);
    disconnectMessage["type"] = "DISCONNECT";
    sendJson(disconnectMessage);

    socket->disconnectFromHost();
    delete socket;
    close();
}

void Client::sendMsg()
{
    QString msg = mainDialog->messageEdit->text();
    QJsonObject jsonMessage;
    jsonMessage["type"] = "MESSAGE";
    jsonMessage["message"] = msg;
    sendJson(jsonMessage);
    mainDialog->messageEdit->clear();
}

void Client::sendJson(QJsonObject data)
{
//    qDebug() << data;
    QJsonDocument doc(data);
    QByteArray ba = doc.toJson();

    socket->writeDatagram(ba, QHostAddress(ip), port);
}

void Client::processMsg()
{
//    qDebug() << "processing message";
    while (socket->hasPendingDatagrams()) {
        QByteArray ba;
        ba.resize(socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        socket->readDatagram(ba.data(), ba.size(), &sender, &senderPort);

        QJsonDocument doc = QJsonDocument::fromJson(ba);
        if (!doc.isObject()) return;

        QJsonObject jsonObj = doc.object();
        QString type = jsonObj["type"].toString();
        QString message = jsonObj["message"].toString();

        if (type == "WELCOME")
        {
            int clientId = parseClientIdFromMsg(message);
            if (clientId != -1) {
                QColor color = generateColorForClient(clientId);
                qDebug() << "add player " << clientId;
                graphicsDialog->addActivePlayer(clientId, color);
            }
            mainDialog->textBrowser->append("Server: " + message);
        }
        else if (type == "ACTIVE_CLIENTS")
        {
            // process the list of active client IDs received from the server
            QJsonArray clientIdsArray = jsonObj["clientIds"].toArray();

            // set of current active client IDs received from the server
            QSet<int> newActiveClients;
            for (const QJsonValue &value : clientIdsArray) {
                int clientId = value.toInt();
                newActiveClients.insert(clientId);
                QColor color = generateColorForClient(clientId);
                qDebug() << "add active player " << clientId;

                // add player if not already in the graphics dialog
                if (!activeClients.contains(clientId)) {
                    graphicsDialog->addPlayer(clientId, color);
                }
            }

            // check for players that are no longer active
            for (int clientId : activeClients) {
                if (!newActiveClients.contains(clientId)) {
                    qDebug() << "remove active player " << clientId;
                    graphicsDialog->removePlayer(clientId);
                }
            }

            activeClients = newActiveClients;

            mainDialog->textBrowser->append("Server: Active clients updated.");
        }
        else if (type == "POSITION")
        {
            QJsonArray playersArray = jsonObj["players"].toArray();
            graphicsDialog->updatePlayerPositions(playersArray);
        }
        else if (type == "OBSTACLE_POSITION")
        {
            QJsonArray obstaclesArray = jsonObj["obstacles"].toArray();
            graphicsDialog->updateObstaclePositions(obstaclesArray);
        }
        else
        {
            mainDialog->textBrowser->append("Server: " + message);
        }
    }
}


int Client::parseClientIdFromMsg(const QString &msg) {
    QRegExp regex("You are Client (\\d+)");
    if (regex.indexIn(msg) != -1) {
        return regex.cap(1).toInt();
    }
    return -1;
}

QColor Client::generateColorForClient(int clientId) {
    static QList<QColor> colors = {
        QColor("red"), QColor("green"), QColor("blue"),
        QColor("yellow")
    };
    return colors[clientId % colors.size()];
}

void Client::sendPlayerPosition(int clientId, qreal x, qreal y)
{
    qDebug() << "sending player position";

    QJsonObject message;
    message["type"] = "POSITION";
    QJsonArray playerPosArray;

    QJsonObject playerPosData;
    playerPosData["clientId"] = clientId;
    playerPosData["x"] = x;
    playerPosData["y"] = y;

    playerPosArray.append(playerPosData);
    message["players"] = playerPosArray;

    sendJson(message);
}

QString Client::getLocalIpAddress() {
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface &interface : interfaces) {
        if (interface.flags() & QNetworkInterface::IsUp) {
            QList<QNetworkAddressEntry> entries = interface.addressEntries();

            for (const QNetworkAddressEntry &entry : entries) {
                QHostAddress ip = entry.ip();

                if (ip.protocol() == QAbstractSocket::IPv4Protocol && ip != QHostAddress("127.0.0.1")) {
                    return ip.toString();
                }
            }
        }
    }
    return QString();
}

void Client::setLocalIpAddress() {
    QString localIp = getLocalIpAddress();
    if (!localIp.isEmpty()) {
        mainDialog->ipEdit->setText(localIp);
    }
}
