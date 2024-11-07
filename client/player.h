#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsItem>
#include <QPainter>
#include <QObject>

class Player : public QObject, public QGraphicsItem {
    Q_OBJECT
public:
    Player(int id, QColor color, QGraphicsItem *parent = nullptr);

    int clientId;
    QColor color;
    qreal x, y;

    void goLeft();
    void goRight();
    void goUp();
    void goDown();
    void stop();

    void setPos(qreal x, qreal y);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void checkCollisionWithObstacles(const QList<QGraphicsItem *> &obstacles);
signals:
    void positionChanged();

};

#endif // PLAYER_H
