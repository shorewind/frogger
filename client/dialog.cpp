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

    ui->greenButton->setStyleSheet("background-color: green");
    ui->blueButton->setStyleSheet("background-color: blue");
    ui->yellowButton->setStyleSheet("background-color: yellow");
    ui->redButton->setStyleSheet("background-color: red");

    connect(ui->ipEdit, &QLineEdit::textChanged, this, &Dialog::updateConnectButtonState);
    connect(ui->portEdit, &QLineEdit::textChanged, this, &Dialog::updateConnectButtonState);
    updateConnectButtonState();

    connect(ui->connectButton, &QPushButton::clicked, this, &Dialog::connectToServer);
    connect(ui->submitButton, &QPushButton::clicked, this, &Dialog::submitUsername);
    connect(ui->greenButton, &QPushButton::clicked, this, &Dialog::onColorButtonClick);
    connect(ui->blueButton, &QPushButton::clicked, this, &Dialog::onColorButtonClick);
    connect(ui->yellowButton, &QPushButton::clicked, this, &Dialog::onColorButtonClick);
    connect(ui->redButton, &QPushButton::clicked, this, &Dialog::onColorButtonClick);
}

void Dialog::updateConnectButtonState()
{
    // enable connect button if both fields have text, else disable it
    bool ipHasText = !ui->ipEdit->text().isEmpty();
    bool portHasText = !ui->portEdit->text().isEmpty();

    ui->connectButton->setEnabled(ipHasText && portHasText);
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
        qDebug() << "socket valid";
        ui->textBrowser->append("Connected to Server: " + ip + ":" + QString::number(port));

        QJsonObject connectMessage;
        connectMessage["type"] = "CONNECT";

        sendJson(connectMessage);

        connect(ui->sendButton, &QPushButton::clicked, this, &Dialog::sendMsg);
    }
    else
    {
        qDebug() << "invalid socket";
    }
}

void Dialog::closeEvent(QCloseEvent *event)
{
    QJsonObject disconnectMessage;

    disconnectMessage["type"] = "DISCONNECT";
    sendJson(disconnectMessage);

    socket->disconnectFromHost();
    delete socket;
    close();
    event->accept();
}

void Dialog::leaveGame()
{ 
    QJsonObject disconnectMessage;

    disconnectMessage["type"] = "LEAVE";
    sendJson(disconnectMessage);

    if (graphicsDialog)
    {
        graphicsDialog->removePlayer(activeClientId);
        graphicsDialog->close();
        delete graphicsDialog;
        graphicsDialog = nullptr;
    }
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
    while (socket->hasPendingDatagrams())
    {
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

        qDebug() << jsonObj;

        if (type == "WELCOME")
        {
            activeClientId = parseClientIdFromMsg(message);
            ui->textBrowser->append("Server: " + message);
            ui->sendButton->setEnabled(true);
            ui->connectButton->setEnabled(false);
            ui->submitButton->setEnabled(true);
            ui->greenButton->setEnabled(true);
            ui->blueButton->setEnabled(true);
            ui->yellowButton->setEnabled(true);
            ui->redButton->setEnabled(true);
        }
        else if (type == "START")
        {
            if (!graphicsDialog)
            {
                graphicsDialog = new GraphicsDialog(this);
                graphicsDialog->show();
                connect(graphicsDialog, &GraphicsDialog::requestClose, this, &Dialog::leaveGame);
            }

            if (activeClientId != -1 && graphicsDialog)
            {
                QColor color = generateColorForClient(activeClientId);
//                qDebug() << "add player " << activeClientId;
                graphicsDialog->addActivePlayer(activeClientId, color);
            }


        }
        else if (type == "REJECTION")
        {
            qDebug() << "rejection";
            ui->textBrowser->append("Server: " + message);
//            disconnectFromServer();
        }
        else if (type == "ACTIVE_CLIENTS")
        {
            // process the list of active client IDs received from the server
            QJsonArray clientIdsArray = jsonObj["clientIdsInGame"].toArray();

            // set of current active client IDs received from the server
            QSet<int> newActiveClients;
            for (const QJsonValue &value : clientIdsArray)
            {
                int clientId = value.toInt();
                newActiveClients.insert(clientId);
                QColor color = generateColorForClient(clientId);
//                qDebug() << "add active player " << clientId;

                // add player if not already in the graphics dialog
                if (!activeClients.contains(clientId) && graphicsDialog)
                {
                    graphicsDialog->addPlayer(clientId, color);
                }
            }

            // check for players that are no longer active
            for (int clientId : activeClients) {
                if (!newActiveClients.contains(clientId) && graphicsDialog)
                {
//                    qDebug() << "remove active player " << clientId;
                    graphicsDialog->removePlayer(clientId);
                }
            }

            activeClients = newActiveClients;

            ui->textBrowser->append("Server: Active clients updated.");
        }
        else if (type == "POSITION")
        {
//            qDebug() << "updating player positions";
            QJsonArray playersArray = jsonObj["players"].toArray();
            if (graphicsDialog)
            {
                graphicsDialog->updatePlayerPositions(playersArray);
            }
        }
//        else if (type == "OBSTACLE_POSITION")
//        {
//            QJsonArray obstaclesArray = jsonObj["obstacles"].toArray();
//            if (graphicsDialog)
//            {
////                graphicsDialog->updateObstaclePositions(obstaclesArray);
//            }
//        }
        else
        {
            ui->textBrowser->append("Server: " + message);
        }
    }
}

int Dialog::parseClientIdFromMsg(const QString &msg)
{
    QRegExp regex("You are Client (\\d+)");
    if (regex.indexIn(msg) != -1)
    {
        return regex.cap(1).toInt();
    }
    return -1;
}

void Dialog::onColorButtonClick()
{
    QPushButton* button = static_cast<QPushButton*>(sender());

    if (!button) return;

    QString color;

    if (button == ui->greenButton) {
        color = "green";
    } else if (button == ui->blueButton) {
        color = "blue";
    } else if (button == ui->yellowButton) {
        color = "yellow";
    } else if (button == ui->redButton) {
        color = "red";
    }

    setPlayerColor(color);
}

void Dialog::setPlayerColor(QString &color)
{
    playerColor = color;

    QJsonObject colorMessage;

    colorMessage["type"] = "PLAYER_COLOR";
    colorMessage["clientId"] = activeClientId;
    colorMessage["color"] = playerColor;

    sendJson(colorMessage);

    ui->textBrowser->append("You selected the color: " + playerColor);
}

QColor Dialog::generateColorForClient(int clientId)
{
    static QList<QColor> colors = {
        QColor("red"), QColor("green"), QColor("blue"),
        QColor("yellow")
    };
    return colors[clientId % colors.size()];
}

void Dialog::submitUsername()
{
    playerUsername = ui->usernameEdit->text();

    QJsonObject usernameMessage;
    usernameMessage["type"] = "USERNAME";
    usernameMessage["clientId"] = activeClientId;
    usernameMessage["username"] = playerUsername;

    sendJson(usernameMessage);

    ui->textBrowser->append("You submitted the username: " + playerUsername);
}

void Dialog::sendPlayerPosition(int clientId, qreal x, qreal y)
{
//    qDebug() << "sending player position";

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
