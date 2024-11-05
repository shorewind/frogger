#include "dialog.h"
#include "ui_dialog.h"
#include "graphicsdialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

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

        // Send the initial message
//        QByteArray initialMsg = "CONNECT";
//        socket->writeDatagram(initialMsg, QHostAddress(ip), port);

        // Send initial JSON "CONNECT" message
        QJsonObject connectMessage;
        connectMessage["type"] = "CONNECT";
//        connectMessage["message"] = "New Client Connected";

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

    disconnectMessage["type"] = "DISCONNECT";
    sendJson(disconnectMessage);
//    QByteArray disconnectMsg = "DISCONNECT";
//    socket->writeDatagram(disconnectMsg, QHostAddress(ui->ipEdit->text()), ui->portEdit->text().toUInt());
    socket->disconnectFromHost();
    delete socket;
    close();
}

void Dialog::sendMsg()
{
    QString msg = ui->messageEdit->text();
//    QByteArray data = msg.toUtf8();

//    socket->writeDatagram(data, QHostAddress(ui->ipEdit->text()), ui->portEdit->text().toUInt());
    QJsonObject jsonMessage;
    jsonMessage["type"] = "MESSAGE";
    jsonMessage["message"] = msg;
    sendJson(jsonMessage);
    ui->messageEdit->clear();
}

void Dialog::sendJson(QJsonObject data)
{
    QJsonDocument doc(data);
    QByteArray ba = doc.toJson();

    socket->writeDatagram(ba, QHostAddress(ip), port);
}

void Dialog::processMsg()
{
    qDebug() << "processing message";
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

        ui->textBrowser->append("Server: " + message);

        if (type == "WELCOME")
        {
            int clientId = parseClientIdFromMsg(message);
            if (clientId != -1) {
                QColor color = generateColorForClient(clientId);
                graphicsDialog->addPlayer(clientId, color);
            }
        }
        else if (type == "MESSAGE")
        {
        }
    }
}


int Dialog::parseClientIdFromMsg(const QString &msg) {
    QRegExp regex("You are Client (\\d+)");
    if (regex.indexIn(msg) != -1) {
        return regex.cap(1).toInt();
    }
    return -1;  // Return -1 if parsing fails
}

QColor Dialog::generateColorForClient(int clientId) {
    static QList<QColor> colors = {
        QColor("red"), QColor("green"), QColor("blue"),
        QColor("yellow")
    };
    return colors[clientId % colors.size()];  // Cycle through colors based on clientId
}

//void Dialog::sendPlayerPosition(int clientId, qreal x, qreal y) {
//    QJsonObject json;
//    json["clientId"] = clientId;
//    json["x"] = x;
//    json["y"] = y;

//    QJsonDocument doc(json);
//    QByteArray data = doc.toJson();

//    QString ip = ui->ipEdit->text();
//    quint16 port = ui->portEdit->text().toUInt();
//    socket->writeDatagram(data, QHostAddress(ip), port);
//}


Dialog::~Dialog()
{
    delete ui;
}
