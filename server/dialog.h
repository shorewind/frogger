#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QtDebug>
#include <QList>
#include <QHostAddress>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMap>
#include <QPoint>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

private:
    Ui::Dialog *ui;

    QUdpSocket* socket;
    QNetworkDatagram msg;
    QList<QHostAddress> clientAddresses;
    QList<quint16> clientPorts;
    QList<int> availableIds;
    QMap<QString, int> clientIdMap;
    QMap<int, QPoint> playerPositions;
    QJsonArray playersArray;

private slots:
    void rx();  // receive
    void tx(QJsonObject message);  // transmit
    void configureServer();
    void removeClient(QString& clientKey);
    void updatePlayerPositions(QJsonArray playersArray);
    void broadcastPlayerPositions();
    void broadcastActiveClients();
};

#endif // DIALOG_H
