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

    // Initialize obstacles in right lanes
    // Row 1 - Skylines (blue cars) - both moving left
    skyline1 = new Obstacle(Obstacle::Skyline, SCENE_WIDTH / 2 - 150, 175, -4, true); // Left-moving car
    skyline2 = new Obstacle(Obstacle::Skyline, SCENE_WIDTH / 2 + 150, 175, -4, true); // Left-moving car

    // Row 2 - Supras (orange cars) - both moving right
    supra1 = new Obstacle(Obstacle::Supra, -SCENE_WIDTH / 2 + 150, 100, 4); // Right-moving car
    supra2 = new Obstacle(Obstacle::Supra, -SCENE_WIDTH / 2 + 350, 100, 4); // Right-moving car

    // Row 3 - Chargers (grey cars) - both moving left
    charger1 = new Obstacle(Obstacle::Charger, SCENE_WIDTH / 2 - 150, 25, -4, true); // Left-moving car
    charger2 = new Obstacle(Obstacle::Charger, SCENE_WIDTH / 2 + 150, 25, -4, true); // Left-moving car
    charger3 = new Obstacle(Obstacle::Charger, SCENE_WIDTH / 2 + 450, 25, -4, true); // Left-moving car
    charger4 = new Obstacle(Obstacle::Charger, SCENE_WIDTH / 2 + 450, 25, -4, true); // Left-moving car

    // calls function to draw score display
    drawScoreDisplay();


    // Add all cars to scene
    scene->addItem(charger1);  // These are now Chargers
    scene->addItem(charger2);
    scene->addItem(charger3);
    scene->addItem(charger4);
    scene->addItem(supra1);
    scene->addItem(supra2);
    scene->addItem(skyline1);
    scene->addItem(skyline2);

    //obstacle list
    obstacleList.append(charger1);
    obstacleList.append(charger2);
    obstacleList.append(charger3);
    obstacleList.append(charger4);
    obstacleList.append(supra1);
    obstacleList.append(supra2);
    obstacleList.append(skyline1);
    obstacleList.append(skyline2);

    // Start moving all cars
    charger1->startMoving();
    charger2->startMoving();
    charger3->startMoving();
    charger4->startMoving();
    supra1->startMoving();
    supra2->startMoving();
    skyline1->startMoving();
    skyline2->startMoving();

    // Spawn logs
    // Row 1: short long short moving left
    log1 = new Obstacle(Obstacle::SHORTW, SCENE_WIDTH / 2 - 100, -50, -2, false);
    log2 = new Obstacle(Obstacle::SHORTW, SCENE_WIDTH / 2 + 110, -50, -2, false);

    scene->addItem(log1);
    obstacleList.append(log1);
    log1->startMoving();

    scene->addItem(log2);
    obstacleList.append(log2);
    log2->startMoving();


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

    qDebug() << activePlayer->clientId;

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
            qDebug() << "client Id " << clientId << " " << x << ", " << y;
        }

    }
}

void GraphicsDialog::drawScoreDisplay()
{
    // Adds SCORE header and display in top left corner
    // outside scene perimeter
    QFont scoreFont("Georgia",18, QFont::Bold);

    // Show score header
    header = new QGraphicsTextItem("SCORE");
    header->setDefaultTextColor(Qt::yellow);
    header->setFont(scoreFont);
    header->setPos(-260, -286);
    // adds the header to scene
    scene->addItem(header);

    // Show score display
    //positions the Score variable just under the position of the word SCORE
    display = new QGraphicsTextItem(QString::number(score));
    display->setDefaultTextColor(Qt::yellow);
    display->setFont(scoreFont);
    display->setPos(-155, -286);
    scene->addItem(display);
}

void GraphicsDialog::closeEvent(QCloseEvent *event) {
    emit requestClose();
    event->accept();
}

void GraphicsDialog::addActivePlayer(int clientId, const QColor &color) {
    if (clientPlayers.contains(clientId)) { return; }

    activePlayer = new Player(clientId, color);
    activePlayer->setPos(-SCENE_WIDTH/2 + clientId * position_width, SCENE_HEIGHT/2 - position_height); // DONE, Adjust position as needed
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
