#ifndef SERVER_H
#define SERVER_H

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
#include <QNetworkInterface>
#include <QTextBrowser>
#include <QLineEdit>

class Dialog;

class Server : public QDialog
{
    Q_OBJECT

public:
    explicit Server(Dialog *parent = nullptr);
    void configureServer();

private:
    Dialog* mainDialog;

    QUdpSocket* socket;
    QNetworkDatagram msg;
    QList<QHostAddress> clientAddresses;
    QList<quint16> clientPorts;
    QList<int> availableIds;
    QMap<QString, int> clientIdMap;
    QMap<int, QPoint> playerPositions;
    QJsonArray playersArray;
    QJsonArray obstaclesArray;

private slots:
    void rx();  // receive
    void tx(QJsonObject message);  // transmit
    void removeClient(QString& clientKey);
    void updatePlayerPositions(QJsonArray playersArray);
    void broadcastPlayerPositions();
    void broadcastActiveClients();
    void broadcastObstaclePositions();
    QString getLocalIpAddress();
    void setLocalIpAddress();
};

#endif // SERVER_H
