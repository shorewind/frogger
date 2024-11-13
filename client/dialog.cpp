#include "dialog.h"
#include "ui_dialog.h"
#include "graphicsdialog.h"
#include "defs.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    setLocalIpAddress();
    ui->portEdit->setText(QString::number(DEFAULT_PORT));  // default port set manually

    connect(ui->connectButton, &QPushButton::clicked, this, &Dialog::connectToServer);
}


void Dialog::connectToServer()
{
    socket = new QUdpSocket(this);

    ip = ui->ipEdit->text();
    port = ui->portEdit->text().toUInt();

    socket->connectToHost(QHostAddress(ip), port);
    connect(socket, &QUdpSocket::readyRead, this, &Dialog::processMsg);

    ui->ipEdit->clear();
    ui->portEdit->clear();

    if (socket->isValid())
    {
        ui->textBrowser->append("Connected to Server: " + ip + ":" + QString::number(port));
        ui->sendButton->setEnabled(true);
        ui->connectButton->setEnabled(false);

        // Send initial JSON "CONNECT" message
        QJsonObject connectMessage;
        connectMessage["type"] = "CONNECT";

        sendJson(connectMessage);

        connect(ui->sendButton, &QPushButton::clicked, this, &Dialog::sendMsg);

        graphicsDialog = new GraphicsDialog(this);
        graphicsDialog->show();
        connect(graphicsDialog, &GraphicsDialog::requestClose, this, &Dialog::disconnectFromServer);
    }
}

void Dialog::closeEvent(QCloseEvent *event)
{
    disconnectFromServer();
    event->accept();
}

void Dialog::disconnectFromServer()
{
    QJsonObject disconnectMessage;

    graphicsDialog->removePlayer(activeClientId);
    disconnectMessage["type"] = "DISCONNECT";
    sendJson(disconnectMessage);

    socket->disconnectFromHost();
    delete socket;
    close();
}

void Dialog::sendMsg()
{
    QString msg = ui->messageEdit->text();
    QJsonObject jsonMessage;
    jsonMessage["type"] = "MESSAGE";
    jsonMessage["message"] = msg;
    sendJson(jsonMessage);
    ui->messageEdit->clear();
}

void Dialog::sendJson(QJsonObject data)
{
//    qDebug() << data;
    QJsonDocument doc(data);
    QByteArray ba = doc.toJson();

    socket->writeDatagram(ba, QHostAddress(ip), port);
}

void Dialog::processMsg()
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
            ui->textBrowser->append("Server: " + message);
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

            ui->textBrowser->append("Server: Active clients updated.");
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
            ui->textBrowser->append("Server: " + message);
        }
    }
}


int Dialog::parseClientIdFromMsg(const QString &msg) {
    QRegExp regex("You are Client (\\d+)");
    if (regex.indexIn(msg) != -1) {
        return regex.cap(1).toInt();
    }
    return -1;
}

QColor Dialog::generateColorForClient(int clientId) {
    static QList<QColor> colors = {
        QColor("red"), QColor("green"), QColor("blue"),
        QColor("yellow")
    };
    return colors[clientId % colors.size()];
}

void Dialog::sendPlayerPosition(int clientId, qreal x, qreal y)
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

QString Dialog::getLocalIpAddress() {
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

void Dialog::setLocalIpAddress() {
    QString localIp = getLocalIpAddress();
    if (!localIp.isEmpty()) {
        ui->ipEdit->setText(localIp);
    }
}

Dialog::~Dialog()
{
    delete ui;
}
