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
#include <QNetworkInterface>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFontDatabase>
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

    QUdpSocket* socket = nullptr;
    QNetworkDatagram msg;
    QList<QHostAddress> clientAddresses;
    QList<quint16> clientPorts;
    QList<int> availableIds;
    QList<int> clientIdsInGame;
    QList<int>playersFinished;
    QMap<QString, ClientData> clientIdMap;  // holds clientKey, clientId, and in_game bool
    QMap<int, QJsonObject> playerPositions;
    QJsonArray playersArray;
    QJsonArray obstaclesArray;
    bool activeGame;
    bool roundOver = false;
    bool gameOver = false;
    QSqlDatabase db;
    QSqlQuery query;
    int currentGameId;
    int currentSessionId;

    void logGame();
    void sendGameData();
    void checkGameState();
    void removeClient(QString& clientKey);
    void updatePlayerPositions(QJsonArray playersArray);
    void broadcastPlayerPositions();
    void broadcastActiveClients();
    void broadcastObstaclePositions();
    QString getLocalIpAddress();
    void setLocalIpAddress();
    void setupDatabase();
    void setActiveClient(ClientData clientData);
    void removeActiveClient(ClientData clientData);
    void kickClient();
    QString getClientKeyFromClientId(int clientId);

private slots:
    void rx();  // receive
    void tx(QJsonObject message);  // transmit
    void configureServer();
    void startGame();
};

#endif // DIALOG_H
