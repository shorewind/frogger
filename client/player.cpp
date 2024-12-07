#include "player.h"
#include "defs.h"

Player::Player(int id, QString username, QColor color, QGraphicsItem *parent)
    : QGraphicsItem(parent), clientId(id), username(username), color(color), x(0), y(0) {
    onLog = false;
    finished = false;
    dead = false;
    inGame = false;
    angle = 0;
}

void Player::goLeft()
{
    angle = -90;
    if (x - PLAYER_WIDTH > -SCENE_WIDTH / 2) // check left boundary
        setPos(x - MOVE_OFFSET, y);
}

void Player::goRight()
{
    angle = 90;
    if (x + PLAYER_WIDTH < SCENE_WIDTH / 2) // check right boundary
        setPos(x + MOVE_OFFSET, y);
}

void Player::goUp()
{
    angle = 0;
    if (y - PLAYER_HEIGHT > -SCENE_HEIGHT / 2) // check top boundary
        setPos(x, y - MOVE_OFFSET);
}

void Player::goDown()
{
    angle = 180;
    if (y + PLAYER_HEIGHT < SCENE_HEIGHT / 2) // check bottom boundary
        setPos(x, y + MOVE_OFFSET);
}

void Player::stop()
{
    return;
}

void Player::setPos(int newX, int newY)
{
    x = newX;
    y = newY;

    QGraphicsItem::setPos(x, y);
    update();  // request a repaint
    emit positionChanged();
}

// for repainting
QRectF Player::boundingRect() const
{
    int maxWidth = std::max(PLAYER_WIDTH, textWidth);
    return QRectF(-maxWidth/2, -PLAYER_HEIGHT/2, maxWidth, PLAYER_HEIGHT+textHeight+USERTEXT_PADDING);
}

void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // debug boundingRect vs shape
//    painter->setPen(QPen(Qt::red, 1, Qt::DashLine));
//    painter->drawRect(boundingRect());

//    painter->setPen(QPen(Qt::blue, 1, Qt::DotLine));
//    painter->drawPath(shape());

    QPixmap playerImage;

    if (color == QColor("red"))
    {
        playerImage = QPixmap(":images/red_frog.png");
    }
    else if (color == QColor("blue"))
    {
        playerImage = QPixmap(":images/blue_frog.png");
    }
    else if (color == QColor("green"))
    {
        playerImage = QPixmap(":images/green_frog.png");
    }
    else if (color == QColor("yellow"))
    {
        playerImage = QPixmap(":images/yellow_frog.png");
    }
    else
    {
        playerImage = QPixmap(":images/white_frog.png");
    }

    if (!playerImage.isNull())
    {
        playerImage = playerImage.scaled(PLAYER_WIDTH, PLAYER_HEIGHT, Qt::KeepAspectRatio);
        painter->save();
        painter->rotate(angle);
        painter->drawPixmap(-PLAYER_WIDTH / 2, -PLAYER_HEIGHT / 2, playerImage);
        painter->restore();
    }
    else  // default rectangle
    {
        painter->setBrush(color);
        painter->drawRect(-PLAYER_WIDTH/2, -PLAYER_HEIGHT/2, PLAYER_WIDTH, PLAYER_HEIGHT);
    }

    // add player username beneath player icon of size of username text
    painter->setPen(QPen(Qt::white));
    painter->setFont(QFont("Arial", 8));
    QFontMetrics metrics(painter->font());
    textWidth = metrics.horizontalAdvance(username);
    textHeight = metrics.height();
    QRect textRect(-textWidth/2-USERTEXT_PADDING/4, PLAYER_HEIGHT/2+USERTEXT_PADDING, textWidth, textHeight);
    painter->drawText(textRect, Qt::AlignHCenter, username);
}

// for collisions
QPainterPath Player::shape() const
{
    QPainterPath path;
    path.addRect(-PLAYER_WIDTH/2, -PLAYER_HEIGHT/2, PLAYER_WIDTH, PLAYER_HEIGHT);
    return path;
}

void Player::resetPlayerPos()
{
   setPos(-SCENE_WIDTH/2 + clientId * PLAYER_XOFFSET, SCENE_HEIGHT/2 - PLAYER_YOFFSET);
}
