#include "graphicsdialog.h"
#include "defs.h"
#include "obstacles.h"
#include <QTimer>
#include "dialog.h"
//286

// 116 for each hole and a half
// therefore 805 total length
// half of 116 is 58
// 174 correct for hole 2
// 40 fro left side
// a box is 38

// correct vertical start 55
// bottom of line is 38
static float position_width = 174;
static float position_height = 55;


GraphicsDialog::GraphicsDialog(QWidget *parent, QUdpSocket *socket) :
    QDialog(parent),
    scene(new QGraphicsScene(this)),
    activePlayer(nullptr),
    socket(socket) // Initialize the socket
{
    // set up the scene dimensions
    scene->setSceneRect(-SCENE_WIDTH / 2, -SCENE_HEIGHT / 2, SCENE_WIDTH, SCENE_HEIGHT);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setBackgroundBrush(Qt::black);

    // load and set the background image
    QPixmap background(":/images/background.png");
    background = background.scaled(SCENE_WIDTH, SCENE_HEIGHT, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QGraphicsPixmapItem *backgroundItem = new QGraphicsPixmapItem(background);
    backgroundItem->setPos(-SCENE_WIDTH / 2, -SCENE_HEIGHT / 2);
    backgroundItem->setZValue(-1);
    scene->addItem(backgroundItem);

    // create a QGraphicsView and set it to use the scene
    QGraphicsView *view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing);
    view->setFixedSize(SCENE_WIDTH, SCENE_HEIGHT);

    // set layout for the dialog
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(view);
    setLayout(layout);

    // Initialize obstacles using the new createObstacle function

    // Row 1 - Skylines (blue cars) - both moving left
    createObstacle(Obstacle::Skyline, SCENE_WIDTH / 2 - 150, 175, -4, true);
    createObstacle(Obstacle::Skyline, SCENE_WIDTH / 2 + 150, 175, -4, true);

    // Row 2 - Supras (orange cars) - both moving right
    createObstacle(Obstacle::Supra, -SCENE_WIDTH / 2 + 150, 100, 4);
    createObstacle(Obstacle::Supra, -SCENE_WIDTH / 2 + 350, 100, 4);

    // Row 3 - Chargers (grey cars) - both moving left
    createObstacle(Obstacle::Charger, SCENE_WIDTH / 2 - 150, 25, -4, true);
    createObstacle(Obstacle::Charger, SCENE_WIDTH / 2 + 150, 25, -4, true);

    // Add other obstacles to the QMap dynamically
    createObstacle(Obstacle::ShortLog, SCENE_WIDTH / 2 - 100, -50, -2, false);
    createObstacle(Obstacle::ShortLog, SCENE_WIDTH / 2 + 110, -50, -2, false);

    // Start moving all obstacles
    for (auto &obstacle : obstacles) {
        obstacle->startMoving();
    }

     QTimer *collisionTimer = new QTimer(this);
         connect(collisionTimer, &QTimer::timeout, this, &GraphicsDialog::checkCollisions);
         collisionTimer->start(50);
}

void GraphicsDialog::createObstacle(Obstacle::ObstacleType type, int x, int y, int speed, bool facingLeft) {
    Obstacle* newObstacle = new Obstacle(type, x, y, speed, facingLeft);
    newObstacle->id = obstacleId++;
    newObstacle->type = type;
    obstacles.insert(newObstacle->id, newObstacle);
    scene->addItem(newObstacle);

    newObstacle->startMoving();
}

void GraphicsDialog::checkCollisions() {
    for (int clientId : clientPlayers.keys()) {
        Player *player = clientPlayers[clientId];
        if (!player) continue; // Ensure the player is valid

        // Loop through obstacles stored in the QMap
        for (auto &obstacle : obstacles) {
            sendObstaclePositions();
            if (player->collidesWithItem(obstacle)) {
                // Handle collision - reset player position based on client ID
                player->setPos(clientId * 2, 245); // Adjust to your desired reset position
                break; // Exit the loop after handling one collision for this player
            }
        }
    }
}

GraphicsDialog::~GraphicsDialog() {
    delete scene;
}

void GraphicsDialog::keyPressEvent(QKeyEvent *e)
{
    // Assuming obstacleList is a QList<QGraphicsItem *> containing the car obstacles
    if (!activePlayer) {
        return;
    }
    switch (e->key())
    {
        case Qt::Key_A:
            activePlayer->goLeft();
            activePlayer->checkCollisionWithObstacles(obstacleList);
            break;
        case Qt::Key_D:
            activePlayer->goRight();
            activePlayer->checkCollisionWithObstacles(obstacleList);
            break;
        case Qt::Key_W:
            activePlayer->goUp();
            activePlayer->checkCollisionWithObstacles(obstacleList);
            break;
        case Qt::Key_S:
            activePlayer->goDown();
            activePlayer->checkCollisionWithObstacles(obstacleList);
            break;
        default:
            activePlayer->stop();
            break;
    }

    QDialog::keyPressEvent(e); // Pass event to the base class
}

void GraphicsDialog::updatePlayerPositions(QJsonArray &playersArray) {
    for (const QJsonValue &value : playersArray) {
        QJsonObject playerData = value.toObject();
        int clientId = playerData["clientId"].toInt();
        int x = playerData["x"].toInt();
        int y = playerData["y"].toInt();

        if (clientId == activePlayer->clientId)
        {
            continue;
        }

        // update position if the player exists
        if (clientPlayers.contains(clientId)) {
            clientPlayers[clientId]->setPos(x, y);
        }
    }
}

void GraphicsDialog::updateObstaclePositions(QJsonArray &obstaclesArray) {
    // Iterate through the array of obstacles provided in the JSON
    for (const QJsonValue &value : obstaclesArray) {
        QJsonObject obstacleData = value.toObject();

        // Extract the obstacle ID, x, and y coordinates from the JSON object
        int obstacleId = obstacleData["obstacleId"].toInt();
        int x = obstacleData["x"].toInt();
        int y = obstacleData["y"].toInt();

        // Check if the obstacle exists in the obstacles QMap
        if (obstacles.contains(obstacleId)) {
            // Get the obstacle object from the QMap using its ID
            Obstacle* obstacle = obstacles[obstacleId];

            // Update the position of the obstacle in the scene
            obstacle->setPos(x, y);

            // Optional: Log the updated position for debugging
            qDebug() << "Updated obstacle " << obstacleId << " to position: (" << x << ", " << y << ")";
        } else {
            // If the obstacle with this ID doesn't exist, you might want to log or handle it
            qDebug() << "Obstacle with ID " << obstacleId << " not found!";
        }
    }
}


void GraphicsDialog::sendObstaclePositions() {
    qDebug() << "sending all obstacle positions";

     // Create a JSON object to hold the message
     QJsonObject message;
     message["type"] = "OBSTACLE_POSITION";

     // Create a JSON array to store the obstacles' position data
     QJsonArray obstaclePosArray;

     // Iterate through all obstacles in the QMap
     for (auto obstaclePair : obstacles) {
         Obstacle* obstacle = obstaclePair;

         // Create a JSON object for each obstacle's data
         QJsonObject obstaclePosData;
         obstaclePosData["obstacleId"] = obstacle->getId();  // Assuming you have a getter for the obstacle ID
         obstaclePosData["obstacleType"] = obstacle->getType();  // Assuming you have a getter for obstacle type
         obstaclePosData["x"] = obstacle->x();
         obstaclePosData["y"] = obstacle->y();
         obstaclePosData["speed"] = obstacle->getSpeed();  // Assuming you have a getter for speed

         // Add the obstacle data to the array
         obstaclePosArray.append(obstaclePosData);
     }

     // Add the array of obstacle data to the message
     message["obstacles"] = obstaclePosArray;
         // The lambda captures 'this' (GraphicsDialog), and calls sendPlayerPosition from Dialog
     Dialog *parentDialog = qobject_cast<Dialog*>(parent());
     if (parentDialog) {
         parentDialog->sendJson(message);
     }
}

void GraphicsDialog::closeEvent(QCloseEvent *event) {
    emit requestClose();
    event->accept();
}

void GraphicsDialog::addActivePlayer(int clientId, const QColor &color) {
    if (clientPlayers.contains(clientId)) { return; }

    activePlayer = new Player(clientId, color);
    activePlayer->setPos(-SCENE_WIDTH/2 + clientId * position_width, SCENE_HEIGHT/2 - position_height); // Adjust position as needed
    scene->addItem(activePlayer);
    clientPlayers[clientId] = activePlayer;

    qDebug() << "added player " << clientId;
    connect(activePlayer, &Player::positionChanged, this, [this]() {
        // The lambda captures 'this' (GraphicsDialog), and calls sendPlayerPosition from Dialog
        Dialog *parentDialog = qobject_cast<Dialog*>(parent());
        if (parentDialog) {
            parentDialog->sendPlayerPosition(activePlayer->clientId, activePlayer->x, activePlayer->y);
        }
    });
}

void GraphicsDialog::addPlayer(int clientId, const QColor &color) {
    if (clientPlayers.contains(clientId)) { return; }

    Player *player = new Player(clientId, color);
    player->setPos(-SCENE_WIDTH/2 + clientId * position_width, SCENE_HEIGHT/2 - position_height); // Adjust position as needed
    scene->addItem(player);
    clientPlayers[clientId] = player;

    qDebug() << "added player " << clientId;
}

void GraphicsDialog::removePlayer(int clientId) {
    if (clientPlayers.contains(clientId)) {
        qDebug() << "removing player " << clientId;
        scene->removeItem(clientPlayers[clientId]);
        delete clientPlayers[clientId];
        clientPlayers.remove(clientId);
    }
}
