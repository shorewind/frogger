#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
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
#include "obstacles.h"

class GraphicsDialog : public QDialog {
    Q_OBJECT

public:
    explicit GraphicsDialog(QWidget *parent = nullptr, QUdpSocket *socket = nullptr);
    ~GraphicsDialog();
    void setPlayerState(QJsonObject clientData);
    void handleLevelOver();
    void checkRoundOver();

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
    QList<QGraphicsRectItem*> boundingLines;
    void createBoundingLine(int x, int y, int width, int height); // Declare here
    QList<QGraphicsItem *> obstacleList;
    void checkCollisions();
    bool roundOver;
    QList<QGraphicsPixmapItem*> hearts;
    int numLives = 3;
    void initializeHearts();
    void removeHeart();
    void sendScoreToServer();
    int obstacleId = 0;

    bool activeGameState=true;

    QGraphicsTextItem *header,*display, *endText;
    QGraphicsRectItem *overlay;
    int score = 0;
    int level = 1;
    int winnerClientId = 0;

public slots:
    void addActivePlayer(int clientId, QString username, const QColor &color);
    void addPlayer(int clientId, QString username, const QColor &color);
    void removePlayer(int clientId);
    void removePlayerFromScene(int clientId);
    void updatePlayerPositions(QJsonArray &playersArray);
    void updateObstaclePositions(QJsonArray &obstaclesArray);
    void sendObstaclePositions();
    void createObstacle(Obstacle::ObstacleType type, int x, int y, int speed, bool facingLeft = false);
    void drawScoreDisplay();
    void showEndScreen();
    void handlePlayerDeath();
//    void setGameStats(bool isAlive, int score, int level);

signals:
    void requestClose();
};

#endif // GRAPHICS_H
