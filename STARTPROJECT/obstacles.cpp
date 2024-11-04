#include "obstacles.h"

Obstacle::Obstacle(int width, int height, int startX, int startY, int speed, QGraphicsItem *parent)
    : QGraphicsRectItem(parent), speed(speed), startX(startX), startY(startY)
{
    // Set the obstacle's size and color
    setRect(0, 0, width, height);
    setBrush(Qt::white); // Set color for visibility

    // Set the initial position
    setPos(startX, startY);

    // Initialize the movement timer
    movementTimer = new QTimer(this);
    connect(movementTimer, &QTimer::timeout, this, &Obstacle::move);
}

void Obstacle::startMoving()
{
    movementTimer->start(50); // Move every 50 milliseconds
}

void Obstacle::move()
{
    // Move the obstacle to the right
    setPos(x() + speed, y());

    // Reset position if it moves beyond the scene's right edge
    if (x() > 400) { // Assuming the right edge of the scene
        setPos(startX, startY); // Reset to the starting position
    }
}
