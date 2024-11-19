#include "dialog.h"
#include "ui_dialog.h"
#include "defs.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    setLocalIpAddress();
    ui->portEdit->setText(QString::number(5678));

    connect(ui->configureButton, &QPushButton::clicked, this, &Dialog::configureServer);
    connect(ui->startButton, &QPushButton::clicked, this, &Dialog::startGame);

    for (int i = 1; i <= 4; ++i) {
        availableIds.append(i);
    }
}

void Dialog::configureServer()
{
    socket = new QUdpSocket(this);

    QString ip = ui->ipEdit->text();
    quint16 port = ui->portEdit->text().toUInt();

    // bind the socket to the specified IP and port
    bool success = socket->bind(QHostAddress(ip), port);
    if (!success) {
        ui->textBrowser->append("Failed to bind server socket: " + socket->errorString());
        return;
    }

    connect(socket, &QUdpSocket::readyRead, this, &Dialog::rx);

    ui->ipEdit->clear();
    ui->portEdit->clear();

    if (socket->isValid())
    {
        ui->textBrowser->append("Server Active: " + ip + ":" + QString::number(port));
        ui->startButton->setEnabled(true);
    }
}

void Dialog::startGame()
{
    QJsonObject startMessage;
    startMessage["type"] = "START";
    startMessage["message"] = "Starting game...";
    ui->textBrowser->append(startMessage["message"].toString());
    tx(startMessage);

    broadcastActiveClients();
//    broadcastPlayerPositions();
//    broadcastObstaclePositions();  // no longer need to sync obstacles
}


void Dialog::rx()
{
    while (socket->hasPendingDatagrams())
    {
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

        qDebug() << "rx";
        qDebug() << jsonObj;
//        qDebug() << senderAddress;
//        qDebug() << type;

        if (type == "DISCONNECT")
        {
            removeClient(clientKey);
            broadcastActiveClients();
            return;
        }
        else if (type == "CONNECT")
        {
            // handle new connections
            int clientId = -1;
            if (!clientIdMap.contains(clientKey) && !availableIds.isEmpty())
            {
                clientId = availableIds.takeFirst();
                clientIdMap[clientKey] = clientId;
                clientAddresses.append(senderAddress);
                clientPorts.append(senderPort);
            }
            else
            {
                // reject the connection if no IDs are available
                QJsonObject rejectionMessage;
                rejectionMessage["type"] = "REJECTION";
                rejectionMessage["message"] = "Server is full. Try again later.";
                QJsonDocument rejectionDoc(rejectionMessage);
                socket->writeDatagram(rejectionDoc.toJson(), senderAddress, senderPort);
                ui->textBrowser->append("Connection rejected: Server is full.");
                return;
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
            ui->textBrowser->append(broadcastMessage["message"].toString());
            tx(broadcastMessage);
        }
        else if (type == "MESSAGE")
        {
            // handle incoming messages from clients
            QString message = jsonObj["message"].toString();

            if (clientIdMap.contains(clientKey) && !message.isEmpty())
            {
                int clientId = clientIdMap[clientKey];
                ui->textBrowser->append(QString("Client %1 (%2:%3): %4")
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
//        else if (type == "OBSTACLE_POSITION")
//        {
//            obstaclesArray = jsonObj["obstacles"].toArray();
//            broadcastObstaclePositions();
//        }
    }
}

void Dialog::updatePlayerPositions(QJsonArray playersArray)
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

void Dialog::broadcastPlayerPositions()
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

void Dialog::broadcastObstaclePositions()
{
    QJsonObject obstaclePositionMessage;
    obstaclePositionMessage["type"] = "OBSTACLE_POSITION";
    obstaclePositionMessage["obstacles"] = obstaclesArray;
    tx(obstaclePositionMessage);
}

void Dialog::broadcastActiveClients()
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


void Dialog::removeClient(QString &clientKey)
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

            ui->textBrowser->append(disconnectionMsg["message"].toString());
            tx(disconnectionMsg);
        }
    }
}

void Dialog::tx(QJsonObject jsonObject)
{
    qDebug() << "tx";
    qDebug() << jsonObject;

    QJsonDocument doc(jsonObject);
    QByteArray message = doc.toJson();

    for (int i = 0; i < clientAddresses.size(); ++i)
    {
        socket->writeDatagram(message, clientAddresses[i], clientPorts[i]);
    }
}

QString Dialog::getLocalIpAddress()
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

void Dialog::setLocalIpAddress()
{
    QString localIp = getLocalIpAddress();
    if (!localIp.isEmpty())
    {
        ui->ipEdit->setText(localIp);
    }
}


Dialog::~Dialog()
{
    delete ui;
}
