#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <QGraphicsRectItem>
#include <QGraphicsPixmapItem>
#include <QObject>
#include <QTimer>

class Obstacle : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    enum CarType { Charger, Supra, Skyline };

    // Constructor for a basic rectangular obstacle
    explicit Obstacle(int width, int height, int startX, int startY, int speed, QGraphicsItem *parent = nullptr);

    // Constructor for a car-type obstacle
    explicit Obstacle(CarType type, int startX, int startY, int speed, bool facingLeft = false, QGraphicsItem *parent = nullptr);

    void startMoving(); // Starts the obstacle movement

private slots:
    void move(); // Slot to handle continuous movement

private:
    void initializeCar(CarType type, bool facingLeft);

    int speed;
    int startX;
    int startY;
    QTimer *movementTimer;
    static const int CAR_WIDTH = 100;
    static const int CAR_HEIGHT = 45;
};

#endif // OBSTACLE_H
