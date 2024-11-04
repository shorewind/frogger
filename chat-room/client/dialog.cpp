#include "dialog.h"
#include "ui_dialog.h"

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
    }
}

void Dialog::closeEvent(QCloseEvent *event)
{
    QByteArray disconnectMsg = "DISCONNECT";
    socket->writeDatagram(disconnectMsg, QHostAddress(ui->ipEdit->text()), ui->portEdit->text().toUInt());
    socket->disconnectFromHost();
    delete socket;
    event->accept();
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

        QString s(ba);
        ui->textBrowser->append("Server: " + s);
    }
}

Dialog::~Dialog()
{
    delete ui;
}
