#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsItem>
#include <QPainter>

class Player : public QGraphicsItem {
public:
    Player(int id, QColor color, QGraphicsItem *parent = nullptr);

    void goLeft();
    void goRight();
    void goUp();
    void goDown();
    void stop();

    void setPos(qreal x, qreal y);

    int getClientId() const { return clientId; }

    // Override necessary methods from QGraphicsItem
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

signals:
//    void positionChanged(int clientId, int x, int y);

private:
    static const int SPEED = 10; // Movement speed
    int clientId; // Unique ID for each player
    QColor color; // Color of the player
    qreal x, y; // Position of the player
};

#endif // PLAYER_H
