#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsRectItem>

class Player : public QGraphicsRectItem
{
public:
    explicit Player(int sceneWidth, int sceneHeight, QGraphicsItem *parent = nullptr);

    void goLeft();
    void goRight();
    void goUp();
    void goDown();
    void stop();

private:
    int sceneWidth;
    int sceneHeight;
};

#endif // PLAYER_H
