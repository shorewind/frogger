#include "server.h"
#include "dialog.h"
#include "defs.h"

Server::Server(Dialog *parent) :
    QDialog(parent)
{
    mainDialog = parent;

    setLocalIpAddress();
    mainDialog->serverPortEdit->setText(QString::number(5678));

    for (int i = 1; i <= 4; ++i) {
        availableIds.append(i);
    }
}

void Server::configureServer()
{
    socket = new QUdpSocket(this);

    QString ip = mainDialog->serverIpEdit->text();
    quint16 port = mainDialog->serverPortEdit->text().toUInt();

    // bind the socket to the specified IP and port
    bool success = socket->bind(QHostAddress(ip), port);
    if (!success) {
        mainDialog->serverBrowser->append("Failed to bind server socket: " + socket->errorString());
        return;
    }

    connect(socket, &QUdpSocket::readyRead, this, &Server::rx);

    mainDialog->serverIpEdit->clear();
    mainDialog->serverPortEdit->clear();

    if (socket->isValid())
    {
        mainDialog->serverBrowser->append("Server Active: " + ip + ":" + QString::number(port));
        mainDialog->serverBrowser->append("Connect by typing in terminal: nc -u " + ip + " " + QString::number(port));
        mainDialog->configureButton->setEnabled(false);
    }
}


void Server::rx()
{
    while (socket->hasPendingDatagrams())
    {
//        qDebug() << "rx";
        msg = socket->receiveDatagram();

        QString clientIP = msg.senderAddress().toString();
        quint16 senderPort = msg.senderPort();
        QByteArray incomingMsg = msg.data();
        QHostAddress senderAddress = msg.senderAddress();

        QString clientKey = QString("%1:%2").arg(clientIP).arg(senderPort);

        QJsonDocument doc = QJsonDocument::fromJson(incomingMsg);
        if (!doc.isObject()) return;  // check if the received data is a valid JSON object

        QJsonObject jsonObj = doc.object();
        QString type = jsonObj["type"].toString();

//        qDebug() << senderAddress;
//        qDebug() << type;

        if (type == "DISCONNECT") {
            removeClient(clientKey);
            broadcastActiveClients();
            return;
        }
        else if (type == "CONNECT")
        {
            // handle new connections
            int clientId = -1;
            if (!clientIdMap.contains(clientKey) && !availableIds.isEmpty()) {
                clientId = availableIds.takeFirst();
                clientIdMap[clientKey] = clientId;
                clientAddresses.append(senderAddress);
                clientPorts.append(senderPort);
            }

            // notify the new client
            QJsonObject welcomeMessage;
            welcomeMessage["type"] = "WELCOME";
            welcomeMessage["message"] = "Welcome to the server! You are Client " + QString::number(clientId);
            QJsonDocument welcomeDoc(welcomeMessage);
            socket->writeDatagram(welcomeDoc.toJson(), senderAddress, senderPort);

            // broadcast new connection
            QJsonObject broadcastMessage;
            broadcastMessage["type"] = "JOINED";
            broadcastMessage["message"] = QString("Client %1 (%2:%3) has joined the server.")
                    .arg(clientId)
                    .arg(clientIP)
                    .arg(senderPort);
            mainDialog->serverBrowser->append(broadcastMessage["message"].toString());
            tx(broadcastMessage);

            broadcastActiveClients();
            broadcastPlayerPositions();
            broadcastObstaclePositions();
        }
        else if (type == "MESSAGE") {
            // handle incoming messages from clients
            QString message = jsonObj["message"].toString();

            if (clientIdMap.contains(clientKey) && !message.isEmpty()) {
                int clientId = clientIdMap[clientKey];
                mainDialog->serverBrowser->append(QString("Client %1 (%2:%3): %4")
                                         .arg(clientId)
                                         .arg(clientIP)
                                         .arg(senderPort)
                                         .arg(message));

                QJsonObject outgoingMessage;
                outgoingMessage["type"] = "MESSAGE";
                outgoingMessage["message"] = QString("Client %1 (%2:%3)> %4")
                        .arg(clientId)
                        .arg(clientIP)
                        .arg(senderPort)
                        .arg(message);
                tx(outgoingMessage);
            }
        }
        else if (type == "POSITION")
        {
            // handle position updates from the client
            playersArray = jsonObj["players"].toArray();

            updatePlayerPositions(playersArray);
            broadcastPlayerPositions();
        }
        else if (type == "OBSTACLE_POSITION")
        {
            obstaclesArray = jsonObj["obstacles"].toArray();
            broadcastObstaclePositions();
        }
    }
}

void Server::updatePlayerPositions(QJsonArray playersArray)
{
    for (const QJsonValue &value : playersArray) {
        QJsonObject playerData = value.toObject();
        int clientId = playerData["clientId"].toInt();
        int x = playerData["x"].toInt();
        int y = playerData["y"].toInt();

        playerPositions[clientId] = QPoint(x, y);

//        qDebug() << "Updated position for client " << clientId << ": (" << x << ", " << y << ")";
    }
}

void Server::broadcastPlayerPositions()
{
//    qDebug() << "broadcasting player positions";
    QJsonObject positionUpdateMessage;
    positionUpdateMessage["type"] = "POSITION";

    QJsonArray playersArray;

    for (auto it = playerPositions.constBegin(); it != playerPositions.constEnd(); ++it)
    {
        QJsonObject playerData;
        playerData["clientId"] = it.key();
        playerData["x"] = it.value().x();
        playerData["y"] = it.value().y();

        playersArray.append(playerData);
    }

    positionUpdateMessage["players"] = playersArray;

    tx(positionUpdateMessage);
}

void Server::broadcastObstaclePositions()
{
    QJsonObject obstaclePositionMessage;
    obstaclePositionMessage["type"] = "OBSTACLE_POSITION";
    obstaclePositionMessage["obstacles"] = obstaclesArray;
    tx(obstaclePositionMessage);
}

void Server::broadcastActiveClients()
{
    QJsonObject activeClientsMessage;
    activeClientsMessage["type"] = "ACTIVE_CLIENTS";

    QJsonArray activeClientsArray;

    for (auto it = clientIdMap.begin(); it != clientIdMap.end(); ++it)
    {
        activeClientsArray.append(it.value());
    }

    activeClientsMessage["clientIds"] = activeClientsArray;

    tx(activeClientsMessage);
}


void Server::removeClient(QString &clientKey)
{
//    qDebug() << "disconnecting " << clientKey;
    if (clientIdMap.contains(clientKey))
    {
        int clientId = clientIdMap[clientKey];
        clientIdMap.remove(clientKey);

        // WIP: Reset the player position to default value
        playerPositions[clientId] = QPoint(clientId*150 - SCENE_WIDTH/2, SCENE_HEIGHT/2 - 55);

        QStringList parts = clientKey.split(':');

        if (parts.size() == 2)
        {
            QHostAddress address = QHostAddress(parts[0]);
            quint16 port = static_cast<quint16>(parts[1].toUInt());

            int index = clientPorts.indexOf(port);
            if (index != -1)
            {
                clientAddresses.removeAt(index);
                clientPorts.removeAt(index);
                availableIds.append(clientId);
                std::sort(availableIds.begin(), availableIds.end());
            }

            // create a disconnection message to broadcast
            QJsonObject disconnectionMsg;
            disconnectionMsg["type"] = "MESSAGE";
            disconnectionMsg["message"] = QString("Client %1 (%2:%3) has disconnected.")
                                             .arg(clientId)
                                             .arg(address.toString())
                                             .arg(port);

            mainDialog->serverBrowser->append(disconnectionMsg["message"].toString());
            tx(disconnectionMsg);
        }
    }
}

void Server::tx(QJsonObject jsonObject)
{
//    qDebug() << "tx";
//    qDebug() << jsonObject;

    QJsonDocument doc(jsonObject);
    QByteArray message = doc.toJson();

    for (int i = 0; i < clientAddresses.size(); ++i)
    {
        socket->writeDatagram(message, clientAddresses[i], clientPorts[i]);
    }
}

QString Server::getLocalIpAddress()
{
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface &interface : interfaces)
    {
        if (interface.flags() & QNetworkInterface::IsUp)
        {
            QList<QNetworkAddressEntry> entries = interface.addressEntries();
            for (const QNetworkAddressEntry &entry : entries)
            {
                QHostAddress ip = entry.ip();
                if (ip.protocol() == QAbstractSocket::IPv4Protocol && ip != QHostAddress("127.0.0.1"))
                {
                    return ip.toString();
                }
            }
        }
    }
    return QString();
}

void Server::setLocalIpAddress()
{
    QString localIp = getLocalIpAddress();
    if (!localIp.isEmpty())
    {
        mainDialog->serverIpEdit->setText(localIp);
    }
}
