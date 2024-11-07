//
// Custom Player Class
//
// Assumes integers PLAYER_WIDTH and PLAYER_HEIGHT are defined within defs.h
//

#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsItem>
#include <QPixmap>
#include <QPainter>
#include <QtDebug>
#include "defs.h"

// for all images imported
const int NUM_IMAGES = 32;
const int NUM_FRAMES = 8;

// no siginal slot capabilities
class Player : public QGraphicsItem
{
public:
    Player(QGraphicsItem *parent = nullptr);
    ~Player() override;

    // redraws the screen, wants to decide if it wants to repaint the area of the scree.
    // if collision boundaries
     QRectF boundingRect() const override;

     explicit Player(int sceneWidth, int sceneHeight, QGraphicsItem *parent = nullptr);

     // actual collision detection
     QPainterPath shape() const override;

     // actually paints the scene
     void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

     // computes the new location of the object and collision detection for that type of object
     // reimplementing, extra methods added in public interface
     void advance(int phase) override;

     // Provide externally accessible steering interface
     void goLeft();
     void goRight();
     void goUp();
     void goDown();
     void stop();

private:

     int sceneWidth;
     int sceneHeight;

     // array for the pixmaps, images
     QPixmap images[NUM_IMAGES];    // Animation sprites

     // initializing the offset and which index pixmap the image is on
     int index;                     // Index within an image subset
     int offset;                    // Offset within images for direction of travel

//     qreal x = 0.0;                 // Current player position (x,y)
//     qreal y = 0.0;

//     // travels at 4.5 speed
//     qreal player_dx = 4.5;         // Fixed increment of travel (player_dx, player_dy);
//     qreal player_dy = 4.5;         // velocity vector

//     // direction of travel
//     qreal dx = 0.0;                // Desired direction of travel
//     qreal dy = 0.0;
};

#endif // PLAYER_H
