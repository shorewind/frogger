#include "graphicsdialog.h"
#include "defs.h"
#include "obstacles.h"

GraphicsDialog::GraphicsDialog(QWidget *parent, QUdpSocket *socket) :
    QDialog(parent),
    scene(new QGraphicsScene(this)),
    player(nullptr),
    socket(socket) // Initialize the socket
{
    // Set up the scene dimensions
    scene->setSceneRect(-SCENE_WIDTH / 2, -SCENE_HEIGHT / 2, SCENE_WIDTH, SCENE_HEIGHT);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setBackgroundBrush(Qt::black);

    // Load and set the background image
    QPixmap background(":/images/background.png");
    background = background.scaled(SCENE_WIDTH, SCENE_HEIGHT, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QGraphicsPixmapItem *backgroundItem = new QGraphicsPixmapItem(background);
    backgroundItem->setPos(-SCENE_WIDTH / 2, -SCENE_HEIGHT / 2);
    backgroundItem->setZValue(-1);
    scene->addItem(backgroundItem);

    // Create a QGraphicsView and set it to use the scene
    QGraphicsView *view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing);
    view->setFixedSize(SCENE_WIDTH, SCENE_HEIGHT);

    // Set layout for the dialog
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(view);
    setLayout(layout);

    // Create and add the log as an Obstacle object
     log1 = new Obstacle(50, 20, -SCENE_WIDTH / 2, -50, 5); // Width 50, height 20, starting position, and speed
     log2 = new Obstacle(50, 20, -SCENE_WIDTH / 2 - 200, -50, 5); // Second log, slightly offset
     scene->addItem(log1);
     scene->addItem(log2);
     log1->startMoving();
     log2->startMoving();
     car1 = new Car(50, 25, -SCENE_WIDTH / 2, 120, 5); // Second log, slightly offset
     obstacleList.append(car1);
     scene->addItem(car1);
     car1->startMoving();
}

GraphicsDialog::~GraphicsDialog() {
    delete scene; // Clean up the scene
}

void GraphicsDialog::keyPressEvent(QKeyEvent *e)
{
    // Assuming obstacleList is a QList<QGraphicsItem *> containing the car obstacles
    switch (e->key())
    {
        case Qt::Key_A:
            player->goLeft();
            player->checkCollisionWithObstacles(obstacleList);
            break;
        case Qt::Key_D:
            player->goRight();
            player->checkCollisionWithObstacles(obstacleList);
            break;
        case Qt::Key_W:
            player->goUp();
            player->checkCollisionWithObstacles(obstacleList);
            break;
        case Qt::Key_S:
            player->goDown();
            player->checkCollisionWithObstacles(obstacleList);
            break;
        default:
            player->stop();
            break;
    }

    QDialog::keyPressEvent(e); // Pass event to the base class
}


void GraphicsDialog::closeEvent(QCloseEvent *event) {
    emit requestClose();
    event->accept();
}

void GraphicsDialog::addPlayer(int clientId, const QColor &color) {
    if (clientPlayers.contains(clientId)) return;

    player = new Player(clientId, color);
    player->setPos(clientId * 40, 40); // Adjust position as needed
    scene->addItem(player);
    clientPlayers[clientId] = player;
}

void GraphicsDialog::removePlayer(int clientId) {
    if (clientPlayers.contains(clientId)) {
        scene->removeItem(clientPlayers[clientId]);
        delete clientPlayers[clientId];
        clientPlayers.remove(clientId);
    }
}
