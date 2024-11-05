#include "player.h"
#include <QBrush>

Player::Player(int sceneWidth, int sceneHeight, QGraphicsItem *parent)
    : QGraphicsRectItem(parent), sceneWidth(sceneWidth), sceneHeight(sceneHeight)
{
    setRect(-10, -10, 20, 20); // Rectangle 20x20 for the player
    setBrush(Qt::green); // Color the player green
}

void Player::goLeft()
{
    if (x() - 10 > -sceneWidth / 2) // Check left boundary
        setPos(x() - dx, y());
}

void Player::goRight()
{
    if (x() + 10 < sceneWidth / 2) // Check right boundary
        setPos(x() + dx, y());
}

void Player::goUp()
{
    if (y() - 10 > -sceneHeight / 2) // Check top boundary
        setPos(x(), y() - dy);
}

void Player::goDown()
{
    if (y() + 10 < sceneHeight / 2) // Check bottom boundary
        setPos(x(), y() + dy);
}

void Player::stop()
{
    // No action needed for stop, but the function exists for key event handling
}
