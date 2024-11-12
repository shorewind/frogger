#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <QGraphicsRectItem>
#include <QGraphicsPixmapItem>
#include <QObject>
#include <QTimer>

class Obstacle : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    enum ObstacleType { Rectangle, Charger, Supra, Skyline, ShortLog, LongLog};

    explicit Obstacle(ObstacleType type, int startX, int startY, int speed, bool facingLeft = false, QGraphicsItem *parent = nullptr);

    void startMoving();
    int getId() const { return id; }
    int getType() const { return type; }
    int getSpeed() const { return speed; }

    static const int LONGW = 200;
    static const int SHORTW = 100;
    static const int LOGH = 50;
    int id;        // Unique identifier for each obstacle
    int type;      // Type of obstacle (Charger, Supra, etc.)

private slots:
    void move(); // Slot to handle continuous movement

private:
    void initializeCar(ObstacleType type, bool facingLeft);
    void initializeLog(int length, bool facingLeft);
    void initializeRectangle(int width, int height);
    int speed;     // Speed of the obstacle
    int startX;
    int startY;
    QTimer *movementTimer;
    QTimer *logTimer;
    QTimer *carTimer;
    static const int CAR_WIDTH = 100;
    static const int CAR_HEIGHT = 38;
};

#endif // OBSTACLE_H
