#include "player.h"
#include "defs.h"
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

Player::Player(int id, QString username, QColor color, QGraphicsItem *parent)
    : QGraphicsItem(parent), clientId(id), username(username), color(color), x(0), y(0) {
    onLog = false;
}

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
    return QRectF(-15, -15, 30+textWidth, 30+textHeight); // return the bounding rectangle for the player centered
}

void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(color);
    painter->drawRect(-15, -15, 30, 30);
    painter->setPen(QPen(Qt::white));
    painter->setFont(QFont("Arial", 8));

    // add player username beneath player icon of size of username text
    QFontMetrics metrics(painter->font());
    textWidth = metrics.horizontalAdvance(username);
    textHeight = metrics.height();
    QRect textRect(-textWidth/2, 20, textWidth+5, textHeight);
    painter->drawText(textRect, Qt::AlignCenter, username);
}

QPainterPath Player::shape() const
{
    QPainterPath path;
    path.addRect(-15, -15, 30, 30);
    return path;
}

void Player::resetPlayerPos() {
   setPos(-SCENE_WIDTH/2 + clientId * PLAYER_XOFFSET, SCENE_HEIGHT/2 - PLAYER_YOFFSET); // DONE, Adjust position as needed
}
