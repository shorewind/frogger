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
#include <QNetworkInterface>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlTableModel>
#include "clientdata.h"

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
    void closeEvent(QCloseEvent *event) override;

    QUdpSocket* socket;
    QNetworkDatagram msg;
    QList<QHostAddress> clientAddresses;
    QList<quint16> clientPorts;
    QList<int> availableIds;
    QList<int> clientIdsInGame;
    QMap<QString, ClientData> clientIdMap;  // holds clientKey, clientId, and in_game bool
    QMap<int, QPoint> playerPositions;
    QJsonArray playersArray;
    QJsonArray obstaclesArray;
    bool activeGame;
    QSqlDatabase db;
    QSqlQuery query;
    int currentGameId;
    int currentSessionId;
    void logGame();

private slots:
    void rx();  // receive
    void tx(QJsonObject message);  // transmit
    void configureServer();
    void removeClient(QString& clientKey);
    void updatePlayerPositions(QJsonArray playersArray);
    void broadcastPlayerPositions();
    void broadcastActiveClients();
    void broadcastObstaclePositions();
    QString getLocalIpAddress();
    void setLocalIpAddress();
    void startGame();
    void setupDatabase();
};

#endif // DIALOG_H
