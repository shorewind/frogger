#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QMap>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QUdpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "player.h"
#include "dialog.h"
#include "obstacles.h" // Include the Obstacle class

class GraphicsDialog : public QDialog {
    Q_OBJECT

public:
    explicit GraphicsDialog(QWidget *parent = nullptr, QUdpSocket *socket = nullptr);
    ~GraphicsDialog();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void closeEvent(QCloseEvent *event) override;

private:
    QGraphicsScene *scene;
    Player *activePlayer;
    QMap<int, Player*> clientPlayers;
    QUdpSocket *socket;
    QString serverIp;
    quint16 serverPort;
    QMap<int, Obstacle*> obstacles;
    QList<QGraphicsItem *> obstacleList;
    void checkCollisions();

//    Obstacle *charger1;  // Moving charger car obstacle
//    Obstacle *charger2;  // Moving charger obstacle
//    Obstacle *supra1; // Moving supra car obstacle
//    Obstacle *supra2; // Moving supra car obstacle
//    Obstacle *skyline1; // Moving skyline car obstacle
//    Obstacle *skyline2; // Moving skyline car obstacle

//    Obstacle *log1;
//    Obstacle *log2;

public slots:
    void addActivePlayer(int clientId, const QColor &color);
    void addPlayer(int clientId, const QColor &color);
    void removePlayer(int clientId);
    void updatePlayerPositions(QJsonArray &playersArray);
//    void updateObstaclePositions(QJsonArray &obstaclesArray);

signals:
    void requestClose();
};

#endif // GRAPHICS_H
