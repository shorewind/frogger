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

        if (incomingMsg == "DISCONNECT") {
            removeClient(clientKey); // Handle the client disconnection
            return;
        }

        if (incomingMsg == "CONNECT") {
            int clientId = -1;
            if (!clientIdMap.contains(clientKey) && !availableIds.isEmpty()) {
                clientId = availableIds.takeFirst();
                clientIdMap[clientKey] = clientId;
                clientAddresses.append(senderAddress);
                clientPorts.append(senderPort);
            }

            // Notify the new client about existing clients
            for (const auto& key : clientIdMap.keys()) {
                int existingClientId = clientIdMap[key];
                QByteArray clientInfoMsg = QString("Client %1").arg(existingClientId).toUtf8();
                socket->writeDatagram(clientInfoMsg, senderAddress, senderPort);
            }

            // Display a connection message in the server UI
            ui->textBrowser->append(QString("New client connected: Client %1 (%2:%3)")
                                     .arg(clientId)
                                     .arg(clientIP)
                                     .arg(senderPort));

            QByteArray welcomeMsg = "Welcome to the server! You are Client " + QByteArray::number(clientId);
            socket->writeDatagram(welcomeMsg, senderAddress, senderPort);

            QByteArray broadcastMsg = "Client " + QByteArray::number(clientId) + " has joined the server.";
            tx(broadcastMsg); // Use tx to broadcast to all clients

            return;
        }

        if (clientIdMap.contains(clientKey)) {
            int clientId = clientIdMap[clientKey];

            ui->textBrowser->append(QString("Client %1 (%2:%3): %4")
                                     .arg(clientId)
                                     .arg(clientIP)
                                     .arg(senderPort)
                                     .arg(QString::fromUtf8(incomingMsg)));

            QByteArray outgoingMsg = "Client " + QByteArray::number(clientId) + " (" + clientIP.toUtf8()
                                     + ":" + QByteArray::number(senderPort) + ") > " + incomingMsg;

            if (!incomingMsg.isEmpty()) {
                tx(outgoingMsg); // Use tx to broadcast to all clients
            }
        }
    }
}

void Dialog::removeClient(QString &clientKey)
{
    if (clientIdMap.contains(clientKey)) {
        int clientId = clientIdMap[clientKey];
        clientIdMap.remove(clientKey);

        QStringList parts = clientKey.split(':');
        if (parts.size() == 2) {
            QHostAddress address = QHostAddress(parts[0]);
            quint16 port = static_cast<quint16>(parts[1].toUInt());

            int index = clientAddresses.indexOf(address);
            if (index != -1) {
                clientAddresses.removeAt(index);
                clientPorts.removeAt(index);
                availableIds.append(clientId);
                std::sort(availableIds.begin(), availableIds.end());
            }

            ui->textBrowser->append(QString("Client %1 has disconnected.").arg(clientId));

            QByteArray disconnectionMsg = "Client " + QByteArray::number(clientId) + " has disconnected.";
            tx(disconnectionMsg);
        }
    }
}


void Dialog::tx(QByteArray &message)
{
    for (int i = 0; i < clientAddresses.size(); ++i) {
        socket->writeDatagram(message, clientAddresses[i], clientPorts[i]);
    }
}


Dialog::~Dialog()
{
    delete ui;
}
