#include "obstacles.h"
#include <QPainter>
#include "defs.h"

// Combined constructor for all obstacle types
Obstacle::Obstacle(ObstacleType type, int startX, int startY, int speed, bool facingLeft, QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent), speed(speed), startX(startX), startY(startY)
{
    // Handle different obstacle types
    switch (type) {
        case Rectangle:
            // Simple rectangular obstacle
            initializeRectangle(100, 40);  // Use the same dimension for both width and height
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

    // Set the initial position
    setPos(startX, startY + 10);

    // Start the movement timer
    movementTimer = new QTimer(this);
    connect(movementTimer, &QTimer::timeout, this, &Obstacle::move);
}

// Initialize a basic rectangular obstacle (used for the Rectangle type)
void Obstacle::initializeRectangle(int width, int height) {
    QPixmap rectanglePixmap(width, height);
    rectanglePixmap.fill(Qt::white);  // Fill with white for visibility
    setPixmap(rectanglePixmap);
}

// Initializes the car appearance based on the CarType
void Obstacle::initializeCar(ObstacleType type, bool facingLeft) {
    QString imagePath;
// Added the Images of sall three PNGs for  use int the main program, however this will
    //not merge properly

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

// initialize either a long or short log
void Obstacle::initializeLog(bool facingLeft)
{
    QString imagePath = ":/images/Log.png";
    // log image sixe: 573 x 149

    QPixmap logImage(imagePath);
    if (logImage.isNull())
    {
        // Default case draws a rectangle if image fails
        logImage = QPixmap(LOGW, LOGH);
        logImage.fill(Qt::magenta);
    }

    // Scale the image to the specified size
    logImage = logImage.scaled(LOGW, LOGH, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // Flip log (not absolutely necesary)
    if (facingLeft)
    {
        logImage = logImage.transformed(QTransform().scale(-1, 1));
    }

    setPixmap(logImage);
}

// Starts the obstacle's movement
void Obstacle::startMoving() {
    movementTimer->start(16); // Adjust timer interval as needed
}

// Moves the obstacle based on its speed
void Obstacle::move() {
    // Update the position of the obstacle based on its speed
    setPos(x() + speed, y());

    // Check boundaries and reset position if obstacle moves off-screen
    if (speed < 0 && x() + boundingRect().width() < -SCENE_WIDTH / 2) {
        // If moving left and off the left side of the screen
        setPos(SCENE_WIDTH / 2, y()); // Reset to the right side of the screen
    } else if (speed > 0 && x() > SCENE_WIDTH / 2) {
        // If moving right and off the right side of the screen
        setPos(-SCENE_WIDTH / 2 - boundingRect().width(), y()); // Reset to the left side
    }
}
