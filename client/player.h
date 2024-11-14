#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsItem>
#include <QGraphicsPixmapItem>  // For displaying images
#include <QPainter>
#include <QObject>
#include <QList>

class Player : public QObject, public QGraphicsItem {
    Q_OBJECT
public:
    Player(int id, QColor color, QGraphicsItem *parent = nullptr);

    int clientId;
    bool onLog;
    QColor color;
    qreal x, y;

    void goLeft();
    void goRight();
    void goUp();
    void goDown();
    void stop();
    void setPos(qreal x, qreal y);
//    int getX() const;
//    int getY()const;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void checkCollisionWithObstacles(const QList<QGraphicsItem *> &obstacles);

  //  void decreaseLife();  // Declare the decreaseLife function


signals:
    void positionChanged();

private:
    int lives = 3;  // Initialize lives to 3
    QList<QGraphicsPixmapItem*> hearts;  // List to hold heart icons
};

#endif // PLAYER_H
