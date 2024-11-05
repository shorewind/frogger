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

    // Bind the socket to the specified IP and port
    bool success = socket->bind(QHostAddress(ip), port);
    if (!success) {
        ui->textBrowser->append("Failed to bind server socket: " + socket->errorString());
        return; // Stop if binding fails
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
        msg = socket->receiveDatagram();

        QString clientIP = msg.senderAddress().toString();
        quint16 senderPort = msg.senderPort();
        QByteArray incomingMsg = msg.data();
        QHostAddress senderAddress = msg.senderAddress();

        QString clientKey = QString("%1:%2").arg(clientIP).arg(senderPort);

        QJsonDocument doc = QJsonDocument::fromJson(incomingMsg);
        if (!doc.isObject()) return; // Check if the received data is a valid JSON object

        QJsonObject jsonObj = doc.object();
        QString type = jsonObj["type"].toString();
        QString message = jsonObj["message"].toString();

        if (type == "DISCONNECT") {
            removeClient(clientKey);
            return;
        }
        else if (type == "CONNECT") {
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
            broadcastMessage["type"] = "MESSAGE";
            broadcastMessage["message"] = QString("Client %1 (%2:%3) has joined the server.")
                    .arg(clientId)
                    .arg(clientIP)
                    .arg(senderPort);
            ui->textBrowser->append(broadcastMessage["message"].toString());
            tx(broadcastMessage);
        }
        else if (type == "MESSAGE") {
            // Handle incoming messages from clients
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
        else if (type == "POSITION") {
            // Handle position messages if needed
        }
    }
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
    qDebug() << "transmitting message";
    // Convert JSON object to QByteArray
    QJsonDocument doc(jsonObject);
    QByteArray message = doc.toJson();

    // Broadcast the message to all connected clients
    for (int i = 0; i < clientAddresses.size(); ++i) {
        qDebug() << clientPorts[i];
        socket->writeDatagram(message, clientAddresses[i], clientPorts[i]);
    }
}



Dialog::~Dialog()
{
    delete ui;
}
