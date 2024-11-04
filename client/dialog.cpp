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

    QString ip = ui->ipEdit->text();
    quint16 port = ui->portEdit->text().toUInt();

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
        QByteArray initialMsg = "CONNECT";
        socket->writeDatagram(initialMsg, QHostAddress(ip), port);

        connect(ui->sendButton, &QPushButton::clicked, this, &Dialog::sendMsg);

        // Show graphics dialog after successful connection
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
    QByteArray disconnectMsg = "DISCONNECT";
    socket->writeDatagram(disconnectMsg, QHostAddress(ui->ipEdit->text()), ui->portEdit->text().toUInt());
    socket->disconnectFromHost();
    delete socket;
    close();
}

void Dialog::sendMsg()
{
    QString msg = ui->messageEdit->text();
    QByteArray data = msg.toUtf8();

    socket->writeDatagram(data, QHostAddress(ui->ipEdit->text()), ui->portEdit->text().toUInt());
    ui->messageEdit->clear();
}

void Dialog::processMsg()
{
    while (socket->hasPendingDatagrams()) {
        QByteArray ba;
        ba.resize(socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        socket->readDatagram(ba.data(), ba.size(), &sender, &senderPort);

        QString msg(ba);
        ui->textBrowser->append("Server: " + msg);

        if (msg.startsWith("Client")) {
            int clientId = parseClientIdFromMsg(msg);
            if (clientId != -1) {
                QColor color = generateColorForClient(clientId);
                graphicsDialog->addPlayer(clientId, color);
            }
        }
    }
}

int Dialog::parseClientIdFromMsg(const QString &msg) {
    QRegExp regex("Client (\\d+)");
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


Dialog::~Dialog()
{
    delete ui;
}
