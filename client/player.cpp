#include "player.h"
#include "defs.h"
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

Player::Player(int id, QColor color, QGraphicsItem *parent)
    : QGraphicsItem(parent), clientId(id), color(color), x(0), y(0) {
    onLog = false;
}

//int Player::getX() const{
//    return x;
//}
//int Player::getY()const{
//    return y;
//}

void Player::goLeft() {
    if (x - 30 > -SCENE_WIDTH / 2) // check left boundary
        setPos(x - 38, y);
}

void Player::goRight() {
    if (x + 30 < SCENE_WIDTH / 2) // check right boundary
        setPos(x + 38, y);
}

void Player::goUp() {
    if (y - 30 > -SCENE_HEIGHT / 2) // check top boundary
        setPos(x, y - 38);
}

void Player::goDown() {
    if (y + 30 < SCENE_HEIGHT / 2) // check bottom boundary
        setPos(x, y + 38);
}

void Player::stop() {
    // logic to stop movement can be implemented if needed
}

void Player::setPos(qreal newX, qreal newY) {
    x = newX;
    y = newY;

    QGraphicsItem::setPos(x, y);
    update(); // Request a repaint
    emit positionChanged();
}

QRectF Player::boundingRect() const {
    return QRectF(-15, -15, 30, 30); // return the bounding rectangle for the player centered
}

void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(color);
    painter->drawRect(-15, -15, 30, 30); // player centered at (0,0)
}

void Player::checkCollisionWithObstacles(const QList<QGraphicsItem *> &obstacles) {
    for (auto obstacle : obstacles) {
        if (this->collidesWithItem(obstacle)) {
           // decreaseLife();  // Reduce lives on collision
            if (lives > 0) {
                setPos(clientId * 2, 245); // Adjust position if player still has lives
            }
            return;
        }
    }
}
void Player::resetPlayerPos() {
   setPos(-SCENE_WIDTH/2 + clientId * PLAYER_XOFFSET, SCENE_HEIGHT/2 - PLAYER_YOFFSET); // DONE, Adjust position as needed
}
