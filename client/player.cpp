#include "player.h"
#include "defs.h"
#include <QDebug>

Player::Player(int id, QColor color, QGraphicsItem *parent)
    : QGraphicsItem(parent), clientId(id), color(color), x(0), y(0) {
}

void Player::goLeft() {
    if (x - 30 > -SCENE_WIDTH / 2) // Check left boundary
        setPos(x - 30, y);
}

void Player::goRight() {
    if (x + 30 < SCENE_WIDTH / 2) // Check right boundary
        setPos(x + 30, y);
}

void Player::goUp() {
    if (y - 30 > -SCENE_HEIGHT / 2) // Check top boundary
        setPos(x, y - 30);
}

void Player::goDown() {
    if (y + 30 < SCENE_HEIGHT / 2) // Check bottom boundary
        setPos(x, y + 30);
}

void Player::stop() {
    // Logic to stop movement can be implemented if needed
}

void Player::setPos(qreal newX, qreal newY) {
    // Update the x and y coordinates
    x = newX;
    y = newY;

    // Use the base class setPos to move the item
    QGraphicsItem::setPos(x, y);
    update(); // Request a repaint
}

QRectF Player::boundingRect() const {
    return QRectF(-15, -15, 30, 30); // Return the bounding rectangle for the player centered
}

void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(color); // Set the color
    painter->drawRect(-15, -15, 30, 30); // Draw the rectangle for the player centered at (0,0)
}

void Player::checkCollisionWithObstacles(const QList<QGraphicsItem *> &obstacles) {
    for (auto obstacle : obstacles) {
        if (this->collidesWithItem(obstacle)) {
            // Reset the player to the starting position upon collision
            setPos(0, 0); // Assuming (0, 0) is the starting point; adjust if needed
            break;
        }
    }
}
