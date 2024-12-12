#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsItem>
#include <QPainter>
#include <QDebug>

class Player : public QObject, public QGraphicsItem {
    Q_OBJECT

public:
    Player(int id, QString username, QColor color, QGraphicsItem *parent = nullptr);

    int clientId;
    QString username;
    bool onLog;
    bool finished;
    bool dead;
    bool inGame;
    QColor color;
    int x, y;
    int angle;
    int textWidth;
    int textHeight;

    void goLeft();
    void goRight();
    void goUp();
    void goDown();
    void stop();
    void setPos(int x, int y);
    void resetPlayerPos();

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

signals:
    void positionChanged();

private:
};

#endif // PLAYER_H
