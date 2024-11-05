#include "player.h"

Player::Player(QGraphicsItem *parent)  //: QGraphicsItem(parent)
{
    for(int k = 0; k < NUM_IMAGES; k++)
    {
        QString s = ":/actual-images/tile0" + QString::number(k) + QString(".png");
        images[k] = QPixmap(s);
        images[k] = images[k].scaled(PLAYER_WIDTH, PLAYER_HEIGHT);
    }

    // Start at first image of walking towards viewer pixmaps
    index = 0;
    // image walking toward you, down the screen
    offset = 28;
}

Player::~Player()
{

}

void Player::goLeft()
{
    dx = -player_dx;
    dy = 0.0;       // only going left no change in y coordinate
    offset = 8;     // right subset of images to go left
}

void Player::goRight()
{
    dx = player_dx;
    dy = 0.0;
    offset = 16;
}

void Player::goUp()
{
    dx = 0.0;
    dy = -player_dy;
    offset = 24;
}

void Player::goDown()
{
    dx = 0.0;
    dy = player_dy;
    offset = 0;
}

void Player::stop()
{
    dx = 0.0;
    dy = 0.0;
}

QRectF Player::boundingRect() const
{
    // WRONG CREATES BOXES CENTERED ON TOP OF EACH OTHER AND RESULT IN IMAGE BEING CUT OFF AND IMAGES DRAGS USING WASD
    //return QRectF(-PLAYER_WIDTH/4, -PLAYER_HEIGHT/4, PLAYER_WIDTH/2, PLAYER_HEIGHT/2);


    return QRectF(-PLAYER_WIDTH/2, -PLAYER_HEIGHT/2, PLAYER_WIDTH, PLAYER_HEIGHT);
}

QPainterPath Player::shape() const  //collision detection
{
    QPainterPath path;
    path.addRect(-PLAYER_WIDTH/2, -PLAYER_HEIGHT/2, PLAYER_WIDTH, PLAYER_HEIGHT);


    //WRONG COLLISIONS WILL NOT REGISTER, COLLISION DETECTION DOES NOT WORK
    //path.addRect(0, 0, PLAYER_WIDTH, PLAYER_HEIGHT);

    return path;
}

void Player::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *option,
                   QWidget *widget)
{

    // draws the images, player character
    index = (index + 1) % NUM_FRAMES;
    painter->drawPixmap(-PLAYER_WIDTH/2, -PLAYER_HEIGHT/2, images[offset + index]);

    // redraw the parts that have changed
    painter->setPen(QPen(Qt::white));

    // everything is centered on top of each other
    painter->drawRect(this->boundingRect());
}

void Player::advance(int phase)
{
    if (phase == 0)
    {
        return;
    }


    // Compute new coordinates
    qreal x = this->pos().rx();
    qreal y = this->pos().ry();
    x = x + dx;
    y = y + dy;

    // sets the boundaries
    // Force new coordinates to remain within scene bounds
    if (x < -SCENE_WIDTH/2 + PLAYER_WIDTH/2)
    {
        x = -SCENE_WIDTH/2 + PLAYER_WIDTH/2;
    }
    else if (x > SCENE_WIDTH/2 - PLAYER_WIDTH/2)
    {
        x = SCENE_WIDTH/2 - PLAYER_WIDTH/2;
    }

    if (y < -SCENE_HEIGHT/2 + PLAYER_HEIGHT/2)
    {
        y = -SCENE_HEIGHT/2 + PLAYER_HEIGHT/2;
    }
    else if (y > SCENE_HEIGHT/2 - PLAYER_HEIGHT/2)
    {
        y = SCENE_HEIGHT/2 - PLAYER_HEIGHT/2;
    }

    // Move player to new coordinates
    this->setPos(x, y);

    // Detect any collisions
    QList<QGraphicsItem*> list = collidingItems();
    if (!list.isEmpty())
    {
        qDebug() << "Player detects collision";
    }
}
