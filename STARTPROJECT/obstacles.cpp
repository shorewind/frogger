#include "obstacles.h"

Obstacle::Obstacle()
{
    dx = 20;
    dy = 20;
    width = 80;
    height = 40;

    log = QPixmap("logSprite.png");
}

void Obstacle::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                     QWidget* widget)
{
    painter->drawPixmap(-width/2, -height/2, log);
}

QRectF Obstacle::boundingRect() const
{
    qreal adjust = 0;
    return QRectF(-width/2 - adjust, -height/2 - adjust, width + adjust, height + adjust);
}

void Obstacle::advance(int step)
{
    if (step == 0)
        return;

    QPointF currentLocation = scenePos();
}


















//Obstacle::Obstacle(int width, int height, int startX, int startY, int speed, QGraphicsItem *parent)
//    : QGraphicsRectItem(parent), speed(speed), startX(startX), startY(startY)
//{
//    // Set the obstacle's size and color
//    setRect(0, 0, width, height);
//    setBrush(Qt::white); // Set color for visibility

//    // Set the initial position
//    setPos(startX, startY);

//    // Initialize the movement timer
//    movementTimer = new QTimer(this);
//    connect(movementTimer, &QTimer::timeout, this, &Obstacle::move);
//}

//void Obstacle::startMoving()
//{
//    movementTimer->start(100); // Move every 50 milliseconds
//}

//void Obstacle::move()
//{
//    // Move the obstacle to the right
//    setPos(x() + speed, y());



//    // Reset position if it moves beyond the scene's right edge
//    if (x() > 400) { // Assuming the right edge of the scene
//        setPos(startX, startY); // Reset to the starting position
//    }
//}
