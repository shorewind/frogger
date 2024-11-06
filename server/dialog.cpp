#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    connect(ui->configureButton, SIGNAL(clicked()), this, SLOT(configureServer()));

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
        ui->textBrowser->append("Connect by typing in terminal: nc -u " + ip + " " + QString::number(port));
        ui->configureButton->setEnabled(false);
    }
}


void Dialog::rx()
{
    while (socket->hasPendingDatagrams())
    {
        qDebug() << "rx";
        msg = socket->receiveDatagram();

        QString clientIP = msg.senderAddress().toString();
        quint16 senderPort = msg.senderPort();
        QByteArray incomingMsg = msg.data();
        QHostAddress senderAddress = msg.senderAddress();

        QString clientKey = QString("%1:%2").arg(clientIP).arg(senderPort);
        //    socket->writeDatagram(data, QHostAddress(ui->ipEdit->text()), ui->portEdit->text().toUInt());

        QJsonDocument doc = QJsonDocument::fromJson(incomingMsg);
        if (!doc.isObject()) return;  // check if the received data is a valid JSON object

        QJsonObject jsonObj = doc.object();
        QString type = jsonObj["type"].toString();

        qDebug() << senderAddress;
        qDebug() << type;

        if (type == "DISCONNECT") {
            removeClient(clientKey);
            broadcastActiveClients();
            return;
        }
        else if (type == "CONNECT")
        {
            // Handle new connections
            int clientId = -1;
            if (!clientIdMap.contains(clientKey) && !availableIds.isEmpty()) {
                clientId = availableIds.takeFirst();
                clientIdMap[clientKey] = clientId;
                clientAddresses.append(senderAddress);
                clientPorts.append(senderPort);
            }

            // Notify the new client
            QJsonObject welcomeMessage;
            welcomeMessage["type"] = "WELCOME";
            welcomeMessage["message"] = "Welcome to the server! You are Client " + QString::number(clientId);
            QJsonDocument welcomeDoc(welcomeMessage);
            socket->writeDatagram(welcomeDoc.toJson(), senderAddress, senderPort);

            // Broadcast new connection
            QJsonObject broadcastMessage;
            broadcastMessage["type"] = "JOINED";
            broadcastMessage["message"] = QString("Client %1 (%2:%3) has joined the server.")
                    .arg(clientId)
                    .arg(clientIP)
                    .arg(senderPort);
            ui->textBrowser->append(broadcastMessage["message"].toString());
            tx(broadcastMessage);

            broadcastActiveClients();
        }
        else if (type == "MESSAGE") {
            // Handle incoming messages from clients
            QString message = jsonObj["message"].toString();

            if (clientIdMap.contains(clientKey) && !message.isEmpty()) {
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
            qDebug() << "POSITION received";
            // Handle position updates from the client
            QJsonArray playersArray = jsonObj["players"].toArray();

            // Update player positions
            updatePlayerPositions(playersArray);

            // After updating the positions, broadcast all player positions to all clients
            broadcastPlayerPositions();
        }
    }
}

void Dialog::updatePlayerPositions(const QJsonArray &playersArray)
{
    // Iterate over all player positions received in the "players" array
    for (const QJsonValue &value : playersArray) {
        QJsonObject playerData = value.toObject();
        int clientId = playerData["clientId"].toInt();
        int x = playerData["x"].toInt();
        int y = playerData["y"].toInt();

        // Store or update the player position in the map
        playerPositions[clientId] = QPoint(x, y);

        qDebug() << "Updated position for client " << clientId << ": (" << x << ", " << y << ")";
    }
}

void Dialog::broadcastPlayerPositions()
{
    qDebug() << "broadcasting player positions";
    QJsonObject positionUpdateMessage;
    positionUpdateMessage["type"] = "POSITION";

    // Create a JSON array to store player position data
    QJsonArray playersArray;

    // Iterate through all stored player positions and add them to the array
    for (auto it = playerPositions.constBegin(); it != playerPositions.constEnd(); ++it) {
        QJsonObject playerData;
        playerData["clientId"] = it.key();
        playerData["x"] = it.value().x();
        playerData["y"] = it.value().y();

        playersArray.append(playerData);
    }

    positionUpdateMessage["players"] = playersArray;

    // Broadcast the position update to all clients
    tx(positionUpdateMessage);
}

void Dialog::broadcastActiveClients()
{
    // Create a JSON object to send the active client IDs
    QJsonObject activeClientsMessage;
    activeClientsMessage["type"] = "ACTIVE_CLIENTS";

    // Create a JSON array to store client IDs
    QJsonArray activeClientsArray;

    // Fill the array with the active client IDs
    for (auto it = clientIdMap.begin(); it != clientIdMap.end(); ++it) {
        activeClientsArray.append(it.value());  // Append the clientId
    }

    // Add the client IDs array to the message
    activeClientsMessage["clientIds"] = activeClientsArray;

    // Broadcast the message to all connected clients
    tx(activeClientsMessage);
}



void Dialog::removeClient(QString &clientKey)
{
    qDebug() << "disconnecting " << clientKey;
    if (clientIdMap.contains(clientKey)) {
        int clientId = clientIdMap[clientKey];
        clientIdMap.remove(clientKey);

        QStringList parts = clientKey.split(':');
        if (parts.size() == 2) {
            QHostAddress address = QHostAddress(parts[0]);
            quint16 port = static_cast<quint16>(parts[1].toUInt());

            int index = clientPorts.indexOf(port);
            if (index != -1) {
                clientAddresses.removeAt(index);
                clientPorts.removeAt(index);
                availableIds.append(clientId);
                std::sort(availableIds.begin(), availableIds.end());
            }

            // Create a disconnection message to broadcast
            QJsonObject disconnectionMsg;
            disconnectionMsg["type"] = "MESSAGE";
            disconnectionMsg["message"] = QString("Client %1 (%2:%3) has disconnected.")
                                             .arg(clientId)
                                             .arg(address.toString())
                                             .arg(port);

            ui->textBrowser->append(disconnectionMsg["message"].toString());
            tx(disconnectionMsg); // Broadcast the disconnection message
        }
    }
}



void Dialog::tx(QJsonObject jsonObject)
{
    qDebug() << "tx";
    qDebug() << jsonObject;
    // Convert JSON object to QByteArray
    QJsonDocument doc(jsonObject);
    QByteArray message = doc.toJson();

    // Broadcast the message to all connected clients
    for (int i = 0; i < clientAddresses.size(); ++i) {
//        qDebug() << clientPorts[i];
        socket->writeDatagram(message, clientAddresses[i], clientPorts[i]);
    }
}



Dialog::~Dialog()
{
    delete ui;
}
