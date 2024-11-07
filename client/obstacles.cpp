#include "obstacles.h"
#include <QPainter>
#include "defs.h"

// Constructor for a basic rectangular obstacle, using a pixmap with solid color
Obstacle::Obstacle(int width, int height, int startX, int startY, int speed, QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent), speed(speed), startX(startX), startY(startY)
{
    // Create a solid color pixmap for the rectangle
    QPixmap rectanglePixmap(width, height);
    rectanglePixmap.fill(Qt::white); // Set color for visibility

    setPixmap(rectanglePixmap); // Set as pixmap
    setPos(startX, startY); // Set the initial position

    // Initialize the movement timer
    movementTimer = new QTimer(this);
    connect(movementTimer, &QTimer::timeout, this, &Obstacle::move);
}

// Constructor for a car-type obstacle
Obstacle::Obstacle(CarType type, int startX, int startY, int speed, bool facingLeft, QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent), speed(speed), startX(startX), startY(startY)
{
    initializeCar(type, facingLeft); // Set up the car image and orientation
    setPos(startX, startY);

    // Initialize the movement timer
    movementTimer = new QTimer(this);
    connect(movementTimer, &QTimer::timeout, this, &Obstacle::move);
}

// Initializes the car appearance based on the CarType
void Obstacle::initializeCar(CarType type, bool facingLeft) {
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

