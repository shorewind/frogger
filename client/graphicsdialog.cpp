#include "graphicsdialog.h"
#include "defs.h"
#include "obstacles.h"
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
//static float position_width = 174;
//static float position_height = 55;


GraphicsDialog::GraphicsDialog(QWidget *parent, QUdpSocket *socket) :
    QDialog(parent),
    scene(new QGraphicsScene(this)),
    activePlayer(nullptr),
    socket(socket)
{
    // set up the scene dimensions
    scene->setSceneRect(-SCENE_WIDTH / 2, -SCENE_HEIGHT / 2, SCENE_WIDTH, SCENE_HEIGHT);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setBackgroundBrush(Qt::black);

    // load and set the background image
    QPixmap background(":/images/coolbackground.png");
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

    drawScoreDisplay();
    initializeHearts();

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
    createObstacle(Obstacle::Charger, SCENE_WIDTH / 2 + 450, 25, -4, true);


    // Add other obstacles to the QMap dynamically
    // Row 1
    createObstacle(Obstacle::Log, SCENE_WIDTH / 2 - 150, -45, -2, false);
    createObstacle(Obstacle::Log, SCENE_WIDTH / 2 + 150, -45, -2, false);
    createObstacle(Obstacle::Log, SCENE_WIDTH / 2 + 450, -45, -2, false);
    // Row 2
    createObstacle(Obstacle::Log, SCENE_WIDTH / 2 + 150, -83, 3, false);
    createObstacle(Obstacle::Log, SCENE_WIDTH / 2 - 300, -83, 3, false);
    // Row 3
    createObstacle(Obstacle::Log, SCENE_WIDTH / 2 + 450, -121, -1, false);
    createObstacle(Obstacle::Log, SCENE_WIDTH / 2 + 150, -121, -1, false);
    // Row 4
    createObstacle(Obstacle::Log, SCENE_WIDTH / 2 + 150, -159, 2, false);
    createObstacle(Obstacle::Log, SCENE_WIDTH / 2 - 150, -159, 2, false);
    createObstacle(Obstacle::Log, SCENE_WIDTH / 2 - 450, -159, 2, false);
    // Row 5
    createObstacle(Obstacle::Log, SCENE_WIDTH / 2 - 100, -197, -1, false);
    createObstacle(Obstacle::Log, SCENE_WIDTH / 2 + 100, -197, -1, false);

    sendObstaclePositions();

    QTimer *collisionTimer = new QTimer(this);
    connect(collisionTimer, &QTimer::timeout, this, &GraphicsDialog::checkCollisions);
    collisionTimer->start(16);
}

void GraphicsDialog::createObstacle(Obstacle::ObstacleType type, int x, int y, int speed, bool facingLeft)
{
    Obstacle* newObstacle = new Obstacle(type, x, y, speed, facingLeft);
    if (!newObstacle) {
        qDebug() << "Failed to create obstacle.";
        return;  // Handle any initialization failure
    }

    newObstacle->id = obstacleId++;
    newObstacle->type = type;
    newObstacle->speed = speed;
    obstacles.insert(newObstacle->id, newObstacle);
    scene->addItem(newObstacle);

    newObstacle->startMoving();
}


void GraphicsDialog::initializeHearts()
{
    QPixmap heartPixmap(":/images/heart.gif");
    QPixmap scaledHeartPixmap = heartPixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    for (int i = 0; i < numLives; ++i) {
        QGraphicsPixmapItem *heart = new QGraphicsPixmapItem(scaledHeartPixmap);
        heart->setPos(-SCENE_WIDTH / 2 + 10 + i * 30, -SCENE_HEIGHT / 2 + 10);
        scene->addItem(heart);
        hearts.append(heart);
    }
}

void GraphicsDialog::removeHeart()
{
    if (!hearts.isEmpty())
    {
        QGraphicsPixmapItem *heart = hearts.takeLast();  // Remove the last heart icon from the list
        scene->removeItem(heart);  // Remove it from the scene
        delete heart;  // Free memory
        qDebug() << "Heart removed. Hearts remaining:" << hearts.size();
    }
    else
    {
        qDebug() << "No more hearts to remove.";
    }
}

void GraphicsDialog::checkCollisions() {
    // Loop through all obstacles
    bool collision = false; // This is used to tell that a player PREVIOUSLY was on a log
    for (auto &obstacle : obstacles)
    {
        // If player is colliding with something
        if (activePlayer->collidesWithItem(obstacle))
        {
            // If the obstacle is a LOG
            if (obstacle->type == Obstacle::Log)
            {
                // Move player with the current log
                activePlayer->setPos(activePlayer->x + obstacle->speed, activePlayer->y);
                activePlayer->onLog = true;   // set flag
            }
            else    // Not a log... car or other
            {
                activePlayer->onLog = false;  // set flag
                handlePlayerDeath();
            }
            // Player collided with at least one obstacle
            collision = true;
        }

        // Strategically break early
        if (collision)
        {
            break;
        }
    }


    // Check if two players collide and handle the consequences
    for (auto &otherPlayer : clientPlayers)
    {
        // Player* otherPlayer = activePlayer + 1;

        if(!otherPlayer) continue;
        if (activePlayer != otherPlayer && activePlayer->collidesWithItem(otherPlayer)) {
            // Both players collide, lose a life
            handlePlayerDeath();
            //otherPlayer->handleplayerDeath();

            break;  // Stop after detecting one collision
        }
    }

    // Check if player was on a log, but hopped off.
    if (!collision)
    {
        activePlayer->onLog = false;
    }
    // Log row positions:
    // -21, -59, -97, -135, -173
    // Check if player is in the water kill zone
    qreal y = activePlayer->y;
    if (y == -21 || y == -59 || y == -97 || y == -135 || y == -173)
    {
        if (activePlayer->onLog == false) // if true, kill player or decrease lives
        {
            handlePlayerDeath();
        }
    }
}

void GraphicsDialog::handlePlayerDeath()
{
    if (!activePlayer) return;

    if (numLives > 0) {
        numLives--;
        removeHeart();
        activePlayer->resetPlayerPos();
    }

    // check if game over after removing the heart
    if (numLives == 0 && hearts.isEmpty())
    {
        //activePlayer->setPos(-SCENE_WIDTH/2 + activePlayer->clientId * position_width, SCENE_HEIGHT/2 - position_height);
        activePlayer->resetPlayerPos();
        activeGameState=false;
        qDebug() << "Game Over!";
        showEndScreen();
    }
}

GraphicsDialog::~GraphicsDialog() {
    delete scene;
}

// top is -211 and bottom is 245
void GraphicsDialog::keyPressEvent(QKeyEvent *e)
{
    if (!activePlayer || !activeGameState) { return; }

//    const int hole2X = 174;
//    const int hole2Y = -211;

//    if (activePlayer->getX() == hole2X && activePlayer->getY() == hole2Y){
//        return;
//    }

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
            score = score + 10;
            break;
        case Qt::Key_S:
            activePlayer->goDown();
            activePlayer->checkCollisionWithObstacles(obstacleList);
            break;
        default:
            activePlayer->stop();
            break;
    }

//    if (activePlayer->getX() == hole2X && activePlayer->getY() == hole2Y){
//        activePlayer->stop();
//    }

    display->setPlainText(QString::number(score));
    QDialog::keyPressEvent(e);  // pass event to the base class
}

void GraphicsDialog::updatePlayerPositions(QJsonArray &playersArray)
{
    for (const QJsonValue &value : playersArray)
    {
        QJsonObject playerData = value.toObject();
        int clientId = playerData["clientId"].toInt();
        int x = playerData["x"].toInt();
        int y = playerData["y"].toInt();

        if (clientId == activePlayer->clientId)
        {
            continue;
        }

        // update position if the player exists
        if (clientPlayers.contains(clientId))
        {
            clientPlayers[clientId]->setPos(x, y);
            qDebug() << "position: " << x << ", " << y;

        }
    }
}

void GraphicsDialog::updateObstaclePositions(QJsonArray &obstaclesArray)
{
    for (const QJsonValue &value : obstaclesArray)
    {
        QJsonObject obstacleData = value.toObject();

        int obstacleId = obstacleData["obstacleId"].toInt();
        int x = obstacleData["x"].toInt();
        int y = obstacleData["y"].toInt();
        int speed = obstacleData["speed"].toInt();

        if (obstacles.contains(obstacleId))
        {
            Obstacle* obstacle = obstacles[obstacleId];
            obstacle->setPos(x, y);
            obstacle->speed = speed;

//            qDebug() << "Updated obstacle " << obstacleId << " to position: (" << x << ", " << y << ")";
        } else {
//            qDebug() << "Obstacle with ID " << obstacleId << " not found!";
        }
    }
}

void GraphicsDialog::showEndScreen()
{
    QString OverIm;

    OverIm = ":/images/GameOver.jpg";
    QPixmap im(OverIm);

    im = im.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QGraphicsPixmapItem *backgroundItem = new QGraphicsPixmapItem(im);
    backgroundItem->setPos( -120, -120);  // Position the image at the center
    backgroundItem->setZValue(-1);  // Set Z-value lower than the overlay and text, so it stays in the background
    scene->addItem(backgroundItem);    // Create a semi-transparent overlay using QGraphicsRectItem


    QGraphicsRectItem *overlay = new QGraphicsRectItem(-SCENE_WIDTH / 2, -SCENE_HEIGHT / 2, SCENE_WIDTH, SCENE_HEIGHT);
    overlay->setBrush(QColor(0, 0, 0, 80));  // Semi-transparent black (adjust alpha as needed)
    overlay->setZValue(10);  // Ensure the overlay is above game items

    // Add the overlay to the scene
    scene->addItem(overlay);

    // Add a text label (you can customize the message as needed)
    QGraphicsTextItem *endText = new QGraphicsTextItem("GAME OVER!");
    endText->setDefaultTextColor(Qt::white);
    endText->setFont(QFont("Georgia", 36, QFont::Bold));
    endText->setDefaultTextColor(Qt::red);
    endText->setPos(-150, 0);  // Adjust position as necessary
    endText->setZValue(11);  // Ensure text is above the overlay
    scene->addItem(endText);

    // Optionally, add a button or other interaction elements
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

void GraphicsDialog::sendObstaclePositions()
{
//    qDebug() << "sending all obstacle positions";

     QJsonObject message;
     message["type"] = "OBSTACLE_POSITION";

     QJsonArray obstaclePosArray;

     for (auto obstacle : obstacles)
     {
         QJsonObject obstaclePosData;
         obstaclePosData["obstacleId"] = obstacle->getId();
         obstaclePosData["obstacleType"] = obstacle->getType();
         obstaclePosData["x"] = obstacle->x();
         obstaclePosData["y"] = obstacle->y();
         obstaclePosData["speed"] = obstacle->getSpeed();

         obstaclePosArray.append(obstaclePosData);
     }

     message["obstacles"] = obstaclePosArray;

     Dialog *parentDialog = qobject_cast<Dialog*>(parent());
     if (parentDialog)
     {
         parentDialog->sendJson(message);
     }
}

void GraphicsDialog::closeEvent(QCloseEvent *event)
{
    emit requestClose();
    event->accept();
}

void GraphicsDialog::addActivePlayer(int clientId, const QColor &color)
{
    if (clientPlayers.contains(clientId)) { return; }

    activePlayer = new Player(clientId, color);

    if (!activePlayer) {
        qDebug() << "Failed to create active player.";
        return;  // Early exit if player creation fails
    }

    activePlayer->resetPlayerPos(); // DONE, Adjust position as needed
    scene->addItem(activePlayer);
    clientPlayers[clientId] = activePlayer;

    qDebug() << "added player " << clientId;
    connect(activePlayer, &Player::positionChanged, this, [this]()
        {
        // The lambda captures 'this' (GraphicsDialog), and calls sendPlayerPosition from Dialog
        Dialog *parentDialog = qobject_cast<Dialog*>(parent());
        if (parentDialog) {
            parentDialog->sendPlayerPosition(activePlayer->clientId, activePlayer->x, activePlayer->y);
        }
    });
}

void GraphicsDialog::addPlayer(int clientId, const QColor &color)
{
    if (clientPlayers.contains(clientId)) { return; }

    Player *player = new Player(clientId, color);
    player->resetPlayerPos(); // Adjust position as needed
    scene->addItem(player);
    clientPlayers[clientId] = player;

    qDebug() << "added player " << clientId;
}

void GraphicsDialog::removePlayer(int clientId)
{
    if (clientPlayers.contains(clientId))
    {
        qDebug() << "removing player " << clientId;
        scene->removeItem(clientPlayers[clientId]);
        delete clientPlayers[clientId];
        clientPlayers.remove(clientId);
    }
}
