#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <QGraphicsItem>
#include <QColor>
#include <QPainter>
#include <QRect>
#include <QPointF>



class Obstacle : public QGraphicsItem
{
public:
    explicit Obstacle();
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget);

    void startMoving(); // Starts the obstacle movement


protected:
    void advance(int step);

private:
//    int speed; // Speed of movement
//    int startX; // Starting x-position
//    int startY; // Starting y-position
//    QTimer *movementTimer; // Timer to control movement
    QPixmap log;
    qreal dx, dy;
    qreal width, height;

private slots:
    void move(); // Slot to handle continuous movement
};

#endif // OBSTACLE_H
