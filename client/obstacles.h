#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <QGraphicsPixmapItem>
#include <QTimer>

class Obstacle : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    enum ObstacleType { Rectangle, Charger, Supra, Skyline, Log};

    explicit Obstacle(ObstacleType type, int startX, int startY, int speed, bool facingLeft = false, QGraphicsItem *parent = nullptr);

    void startMoving();

    int id;
    int type;
    int speed;

private slots:
    void move();

private:
    void initializeCar(ObstacleType type, bool facingLeft);
    void initializeLog(bool facingLeft);
    void initializeRectangle(int width, int height);
    int startX;
    int startY;
    QTimer *movementTimer;
};

#endif // OBSTACLE_H
