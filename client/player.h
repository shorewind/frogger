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
    Player(int id, QString username, QColor color, QGraphicsItem *parent = nullptr);

    int clientId;
    QString username;
    bool onLog;
    QColor color;
    qreal x, y;
    int textWidth;
    int textHeight;

    void goLeft();
    void goRight();
    void goUp();
    void goDown();
    void stop();
    void setPos(qreal x, qreal y);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void resetPlayerPos();

signals:
    void positionChanged();

private:
    int lives = 3;  // Initialize lives to 3
    QList<QGraphicsPixmapItem*> hearts;  // List to hold heart icons
};

#endif // PLAYER_H
