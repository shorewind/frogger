#include "obstacles.h"
#include "defs.h"

// combined constructor for all obstacle types
Obstacle::Obstacle(ObstacleType type, int startX, int startY, int speed, bool facingLeft, QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent), speed(speed), startX(startX), startY(startY)
{
    // handle different obstacle types
    switch (type) {
        case Rectangle:
            initializeRectangle(100, 40);
            break;
        case Charger:
        case Supra:
        case Skyline:
            initializeCar(type, facingLeft);
            break;
        case Log:
            initializeLog(facingLeft);
            break;
    }

    // set the initial position
    setPos(startX, startY + 10);

    // start the movement timer
    movementTimer = new QTimer(this);
    connect(movementTimer, &QTimer::timeout, this, &Obstacle::move);
}

// initialize a basic rectangular obstacle
void Obstacle::initializeRectangle(int width, int height)
{
    QPixmap rectanglePixmap(width, height);
    rectanglePixmap.fill(Qt::white);
    setPixmap(rectanglePixmap);
}

// initializes the car appearance based on the CarType
void Obstacle::initializeCar(ObstacleType type, bool facingLeft)
{
    QString imagePath;

    switch(type)
    {
        case Charger:
            imagePath = ":/images/Charger.png";
            break;
        case Supra:
            imagePath = ":/images/Supra.png";
            break;
        case Skyline:
            imagePath = ":/images/Skyline.png";
            break;
        case Log:
        case Rectangle:
            break;
    }

    QPixmap carImage(imagePath);
    if(carImage.isNull())
    {
        // fallback if image loading fails
        carImage = QPixmap(CAR_WIDTH, CAR_HEIGHT);
        carImage.fill(Qt::red);
    }

    // scale the image to desired size
    carImage = carImage.scaled(CAR_WIDTH, CAR_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // flip the image if needed
    if(facingLeft)
    {
        carImage = carImage.transformed(QTransform().scale(-1, 1));
    }

    setPixmap(carImage);
}

void Obstacle::initializeLog(bool facingLeft)
{
    QString imagePath = ":/images/Log.png";
    // log image sixe: 573 x 149

    QPixmap logImage(imagePath);
    if (logImage.isNull())
    {
        // default case draws a rectangle if image fails
        logImage = QPixmap(LOG_WIDTH, LOG_HEIGHT);
        logImage.fill(Qt::magenta);
    }

    // scale the image to the specified size
    logImage = logImage.scaled(LOG_WIDTH, LOG_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // flip log (not absolutely necesary)
    if (facingLeft)
    {
        logImage = logImage.transformed(QTransform().scale(-1, 1));
    }

    setPixmap(logImage);
}

void Obstacle::startMoving()
{
    movementTimer->start(MOVEMENT_INTERVAL);
}

void Obstacle::move()
{
    // update the position of the obstacle based on its speed
    setPos(x() + speed, y());

    // check boundaries and reset position if obstacle moves off-screen
    if (speed < 0 && x() + boundingRect().width() < -SCENE_WIDTH / 2)
    {
        // if moving left and off the left side of the screen
        setPos(SCENE_WIDTH / 2, y());  // reset to the right side of the screen
    }
    else if (speed > 0 && x() > SCENE_WIDTH / 2)
    {
        // if moving right and off the right side of the screen
        setPos(-SCENE_WIDTH / 2 - boundingRect().width(), y());  // reset to the left side
    }
}
