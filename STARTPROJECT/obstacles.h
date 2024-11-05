/*#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <QGraphicsRectItem>
#include <QObject>
#include <QTimer>
#include <QBrush>
#include <QColor>



class Obstacle : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

public:
    explicit Obstacle(int width, int height, int startX, int startY, int speed, QGraphicsItem *parent = nullptr);

    void startMoving(); // Starts the obstacle movement

private slots:
    void move(); // Slot to handle continuous movement

private:
    int speed; // Speed of movement
    int startX; // Starting x-position
    int startY; // Starting y-position
    QTimer *movementTimer; // Timer to control movement
};

#endif // OBSTACLE_H
*/

#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QTimer>
#include <QPixmap>

class Obstacle : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT

public:
    enum CarType { Charger, Supra, Skyline };

    // Constructor for cars
    explicit Obstacle(CarType type, int startX, int startY, int speed, bool facingLeft = false, QGraphicsItem *parent = nullptr);

    // Original constructor for backward compatibility
    explicit Obstacle(int width, int height, int startX, int startY, int speed, QGraphicsItem *parent = nullptr);

    void startMoving();

private slots:
    void move();

private:
    void initializeCar(CarType type, bool facingLeft);

    int speed;
    int startX;
    int startY;
    QTimer *movementTimer;
    static const int CAR_WIDTH = 100;  // Default car width
    static const int CAR_HEIGHT = 50;  // Default car height
};

#endif // OBSTACLE_H
