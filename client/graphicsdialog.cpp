#include "graphicsdialog.h"
#include "defs.h"
#include "obstacles.h"
#include <QTimer>

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

    // Initialize obstacles
    log1 = new Obstacle(Obstacle::Charger, SCENE_WIDTH / 2, -100, -4, true);
    log2 = new Obstacle(Obstacle::Charger, SCENE_WIDTH / 2 + 200, -100, -4, true);
    // Add some Supras going left (negative speed)
    supra1 = new Obstacle(Obstacle::Supra, SCENE_WIDTH / 2, 0, -4, true);
    supra2 = new Obstacle(Obstacle::Supra, SCENE_WIDTH / 2 + 200, 0, -4, true);

    // Add some Skylines going right
    skyline1 = new Obstacle(Obstacle::Skyline, -SCENE_WIDTH / 2, 50, 6);
    skyline2 = new Obstacle(Obstacle::Skyline, -SCENE_WIDTH / 2 - 200, 50, 6);

    // Add all cars to scene
    scene->addItem(log1);  // These are now Chargers
    scene->addItem(log2);
    scene->addItem(supra1);
    scene->addItem(supra2);
    scene->addItem(skyline1);
    scene->addItem(skyline2);

    //obstacle list
    obstacleList.append(supra1);
    obstacleList.append(supra2);
    obstacleList.append(skyline1);
    obstacleList.append(skyline2);

    // Start moving all cars
    log1->startMoving();
    log2->startMoving();
    supra1->startMoving();
    supra2->startMoving();
    skyline1->startMoving();
    skyline2->startMoving();



     QTimer *collisionTimer = new QTimer(this);
         connect(collisionTimer, &QTimer::timeout, this, &GraphicsDialog::checkCollisions);
         collisionTimer->start(50);
}

void GraphicsDialog::checkCollisions() {
    for (int clientId : clientPlayers.keys()) {
        Player *player = clientPlayers[clientId];
        if (!player) continue; // Ensure the player is valid

        for (QGraphicsItem *obstacle : obstacleList) {
            if (player->collidesWithItem(obstacle)) {
                // Handle collision - reset player position based on client ID
                player->setPos(clientId * 40, 250); // Adjust to your desired reset position
                break; // Exit the loop after handling one collision for this player
            }
        }
    }
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
    player->setPos(clientId * 40, 250); // Adjust position as needed
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
