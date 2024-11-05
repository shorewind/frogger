#ifndef OBSTACLE_H
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

// Car class as a type of Obstacle
class Car : public Obstacle
{
public:
    Car(int width, int height, int startX, int startY, int speed, QGraphicsItem *parent = nullptr);
};

#endif // OBSTACLE_H

