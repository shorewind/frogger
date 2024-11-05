/*#include "obstacles.h"

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
Car::Car(int width, int height, int starX, int startY, int speed, QGraphicsItem *parent)
    : Obstacle(width, height, startX, startY, speed, parent)

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
*/

#include "obstacles.h"
#include <QGraphicsScene>

Obstacle::Obstacle(CarType type, int startX, int startY, int speed, bool facingLeft, QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent), speed(speed), startX(startX), startY(startY)
{
    initializeCar(type, facingLeft);

    setPos(startX, startY);

    movementTimer = new QTimer(this);
    connect(movementTimer, &QTimer::timeout, this, &Obstacle::move);
}

// Original constructor maintained for compatibility
Obstacle::Obstacle(int width, int height, int startX, int startY, int speed, QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent), speed(speed), startX(startX), startY(startY)
{
    // Load default charger as fallback
    initializeCar(Charger, false);
    setPos(startX, startY);

    movementTimer = new QTimer(this);
    connect(movementTimer, &QTimer::timeout, this, &Obstacle::move);
}

void Obstacle::initializeCar(CarType type, bool facingLeft)
{
    QString imagePath;
    switch(type) {
        case Charger:
            imagePath = ":/images/Charger.png";
            break;
        case Supra:
            imagePath = ":/images/Supra.png";
            break;
        case Skyline:
            imagePath = ":/images/Skyline.png";
            break;
    }

    QPixmap carImage(imagePath);
    if(carImage.isNull()) {
        // Fallback if image loading fails
        carImage = QPixmap(CAR_WIDTH, CAR_HEIGHT);
        carImage.fill(Qt::red);
    }

    // Scale the image to desired size
    carImage = carImage.scaled(CAR_WIDTH, CAR_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // Flip the image if needed
    if(facingLeft) {
        carImage = carImage.transformed(QTransform().scale(-1, 1));
    }

    setPixmap(carImage);
}

void Obstacle::startMoving()
{
    movementTimer->start(16);  // ~60 FPS for smoother movement
}

void Obstacle::move()
{
    // Move the obstacle
    setPos(x() + speed, y());

    // Get scene boundaries for wrapping
    QGraphicsScene* gameScene = scene();
    if(!gameScene) return;

    qreal sceneWidth = gameScene->width();
    qreal halfWidth = sceneWidth / 2;

    // Wrap around the screen
    if(speed > 0 && x() > halfWidth) {
        setPos(-halfWidth, y());
    }
    else if(speed < 0 && x() < -halfWidth) {
        setPos(halfWidth, y());
    }
}
