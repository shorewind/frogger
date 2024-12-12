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

    // Row 1 - Skylines (blue cars) - both moving left
    createObstacle(Obstacle::Skyline, SCENE_WIDTH / 2 - 150, 175, -4, true);
    createObstacle(Obstacle::Skyline, SCENE_WIDTH / 2 + 150, 175, -4, true);
    // Row 2 - Chargers (grey cars) - both moving left
    createObstacle(Obstacle::Charger, -SCENE_WIDTH / 2 - 150, 135, 4);
    createObstacle(Obstacle::Charger, -SCENE_WIDTH / 2 + 150, 135, 4);
    // Row 3 - Supras (orange cars) - both moving right
    createObstacle(Obstacle::Supra, -SCENE_WIDTH / 2 + 150, 100, -4, true);
    createObstacle(Obstacle::Supra, -SCENE_WIDTH / 2 + 350, 100, -4, true);
    // Row 4 - Skylines (blue cars) - both moving left
    createObstacle(Obstacle::Skyline, -SCENE_WIDTH / 2 - 150, 65, 4);
    createObstacle(Obstacle::Skyline, -SCENE_WIDTH / 2 + 300, 65, 4);
    // Row 5 - Chargers (grey cars) - both moving left
    createObstacle(Obstacle::Charger, SCENE_WIDTH / 2 - 150, 25, -4, true);
    createObstacle(Obstacle::Charger, SCENE_WIDTH / 2 + 150, 25, -4, true);
    createObstacle(Obstacle::Charger, SCENE_WIDTH / 2 + 450, 25, -4, true);

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

    QTimer *collisionTimer = new QTimer(this);
    connect(collisionTimer, &QTimer::timeout, this, &GraphicsDialog::checkCollisions);
    collisionTimer->start(COLLISION_INTERVAL);

    // bounding for the top of the game
    // half log
    createBoundingLine(-SCENE_WIDTH / 2, -SCENE_HEIGHT / 2 + 100, 35, 10);
    //  1st
    createBoundingLine(-SCENE_WIDTH / 2 + 85, -SCENE_HEIGHT / 2 + 100, 65, 10);
    // 2nd
    createBoundingLine(-SCENE_WIDTH / 2 + 195, -SCENE_HEIGHT / 2 + 100, 65, 10);
    // 3rd
    createBoundingLine(-SCENE_WIDTH / 2 + 310, -SCENE_HEIGHT / 2 + 100, 65, 10);
    // 4th
    createBoundingLine(-SCENE_WIDTH / 2 + 425, -SCENE_HEIGHT / 2 + 100, 65, 10);
    // 5th
    createBoundingLine(-SCENE_WIDTH / 2 + 545, -SCENE_HEIGHT / 2 + 100, 65, 10);
    // 6th
    createBoundingLine(-SCENE_WIDTH / 2 + 655, -SCENE_HEIGHT / 2 + 100, 65, 10);
    // 7th
    createBoundingLine(-SCENE_WIDTH / 2 + 770, -SCENE_HEIGHT / 2 + 100, 30, 10);
}

void GraphicsDialog::createBoundingLine(int x, int y, int width, int height)
{
    QGraphicsRectItem* boundingLine = new QGraphicsRectItem(x, y, width, height);
    boundingLine->setBrush(Qt::transparent);
    boundingLine->setPen(Qt::NoPen);
    boundingLine->setZValue(1);
    boundingLine->setData(0, "boundingLine");
    scene->addItem(boundingLine);
    boundingLines.append(boundingLine);
}

void GraphicsDialog::createObstacle(Obstacle::ObstacleType type, int x, int y, int speed, bool facingLeft)
{
    Obstacle* newObstacle = new Obstacle(type, x, y, speed, facingLeft);
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

    for (int i = 0; i < numLives; ++i)
    {
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
        QGraphicsPixmapItem *heart = hearts.takeLast();
        scene->removeItem(heart);
        delete heart;
        qDebug() << "Heart removed. Hearts remaining:" << hearts.size();
    }
    else
    {
        qDebug() << "No more hearts to remove.";
    }
}

void GraphicsDialog::checkCollisions()
{
    QPointF currentPos = activePlayer->pos();

    // get all items colliding with the player
    QList<QGraphicsItem*> collisions = scene->collidingItems(activePlayer);

    for (QGraphicsItem* item : collisions)
    {
        // check if the colliding item is a bounding line using a custom data property
        if (item->data(0).toString() == "boundingLine")
        {
            qDebug() << "Collision with bounding line!";
            activePlayer->setPos(static_cast<int>(currentPos.x())+38, static_cast<int>(currentPos.y())+38);;
            return;
        }
    }

    // check for collisions with obstacles
    bool collision = false;
    for (auto& obstacle : obstacles)
    {
        if (activePlayer->collidesWithItem(obstacle))
        {
            if (obstacle->type == Obstacle::Log)
            {
                // move the player with the log
                activePlayer->setPos(static_cast<int>(currentPos.x()) + obstacle->speed, static_cast<int>(currentPos.y()));
                activePlayer->onLog = true;
            }
            else
            {
                activePlayer->onLog = false;
                qDebug() << "obstacle death";
                handlePlayerDeath();
            }
            collision = true;
            break;
        }
    }

    if (!collision)
    {
        activePlayer->onLog = false;
    }

    // water zone check
    int y = static_cast<int>(currentPos.y());
    if (y == -21 || y == -59 || y == -97 || y == -135 || y == -173)
    {
        if (!activePlayer->onLog) {
            qDebug() << "water death";
            handlePlayerDeath();
        }
    }

    // check for reaching lily pad
    if (y == -211 && activePlayer->finished == false)
    {
        activePlayer->finished = true;
        qDebug() << "Player Finished";
        activeGameState = false;
        sendScoreToServer();
        overlay->setBrush(QColor(0, 0, 0, 0));
        overlay->setBrush(QColor(0, 0, 0, 50));  // Semi-transparent black (adjust alpha as needed)
        endText->setPlainText("YOU FINISHED!");
    }
}

void GraphicsDialog::handlePlayerDeath()
{
    if (numLives > 0)
    {
        numLives--;
        removeHeart();
        activePlayer->resetPlayerPos();
    }

    // check if game over after removing the heart
    if (numLives == 0 && hearts.isEmpty())
    {
        activePlayer->dead = true;
        activePlayer->resetPlayerPos();
        activeGameState=false;
        sendScoreToServer();
        overlay->setBrush(QColor(0, 0, 0, 0));
        overlay->setBrush(QColor(0, 0, 0, 50));  // Semi-transparent black (adjust alpha as needed)
        endText->setPlainText("YOU DIED");
    }
}

GraphicsDialog::~GraphicsDialog()
{
    delete scene;
}

void GraphicsDialog::startNextLevel()
{
//    qDebug() << "Start next round called";
//    for(auto &player : clientPlayers.values())
//    {
        if (!activePlayer->dead)
        {
            qDebug() << "Player not dead";
            activePlayer->finished = false;
            activeGameState = true;
            overlay->setBrush(QColor(0, 0, 0, 0));  // Semi-transparent black (adjust alpha as needed)
            endText->setPlainText("");
            activePlayer->resetPlayerPos();
        }
//    }
}

// top is -211 and bottom is 245
void GraphicsDialog::keyPressEvent(QKeyEvent *e)
{
    if (!activePlayer || !activeGameState) { return; }

    switch (e->key())
    {
       case Qt::Key_A:
       case Qt::Key_J:
           activePlayer->goLeft();
           break;
       case Qt::Key_D:
       case Qt::Key_L:
           activePlayer->goRight();
           break;
       case Qt::Key_W:
       case Qt::Key_I:
           activePlayer->goUp();
              score = score + 10;
           break;
       case Qt::Key_S:
       case Qt::Key_K:
           activePlayer->goDown();
              break;
       default:
           activePlayer->stop();
           break;
    }

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
        int angle = playerData["angle"].toInt();

        if (clientId == activePlayer->clientId)
        {
            continue;
        }

        // update position if the player exists
        if (clientPlayers.contains(clientId))
        {
            clientPlayers[clientId]->setPos(x, y);
            clientPlayers[clientId]->angle = angle;
        }
    }
}

void GraphicsDialog::sendScoreToServer()
{
    QJsonObject scoreMsg;
    scoreMsg["type"] = "SCORE";
    scoreMsg["clientId"] = activePlayer->clientId;
    scoreMsg["score"] = score;
    scoreMsg["isAlive"] = !activePlayer->dead;
    scoreMsg["levelsPlayed"] = level;
    scoreMsg["finishedLastLevel"] = activePlayer->finished;
    scoreMsg["isLevelOver"] = levelOver;

    Dialog *parentDialog = qobject_cast<Dialog*>(parent());
    if (parentDialog)
    {
        scoreMsg["playerUsername"] = parentDialog->playerUsername;
        parentDialog->sendJson(scoreMsg);
    }
}

void GraphicsDialog::handleLevelOver()
{
    //showEndScreen();  // major issues with this... image cannot be modified after initially showing...
    activeGameState = false;
    qDebug() << "Level Over";
    QString lvlMsg = QString("LEVEL %1 OVER").arg(level);
    level++;
    for (auto& obstacle : obstacles.values())
    {
        obstacle->speed *= 1.1;
    }
    endText->setPlainText(lvlMsg);
    QTimer::singleShot(3000, this, &GraphicsDialog::startNextLevel);
}


void GraphicsDialog::handleGameOver(QJsonArray resultsArray)
{
    // Define constants for layout
    const int itemHeight = 50; // Vertical spacing between rows
    const int columnWidth = 400; // Width allocated for each column
    const int maxItemsPerColumn = (scene->height() - 350) / itemHeight; // Adjusted padding
    const int startX = -scene->width() / 2 + 50; // Start near the left edge of the scene
    const int startY = -scene->height() / 2 + 200; // Start further down the scene

    int column = 0; // Current column index
    int row = 0; // Current row index

    // Column widths for alignment
    const int placementWidth = 10;
    const int usernameWidth = 20;
    const int scoreWidth = 10;

    // Use a monospaced font for alignment
    QFont font("Courier", 24, QFont::Bold);

    // Add a header row
    QString header = QString("%1%2%3")
                     .arg("Placement", -placementWidth)
                     .arg("Username", -usernameWidth)
                     .arg("Score", -scoreWidth);
    QGraphicsTextItem *headerItem = new QGraphicsTextItem(header);
    headerItem->setFont(font);
    headerItem->setDefaultTextColor(Qt::yellow);
    headerItem->setZValue(12);
    headerItem->setPos(startX, startY - itemHeight); // Position header above the first row
    scene->addItem(headerItem);

    // Iterate through results and add text items
    for (const QJsonValue &value : resultsArray)
    {
        QJsonObject playerResult = value.toObject();
        QString text = QString("%1%2%3")
                       .arg(QString::number(playerResult["placement"].toInt()), -placementWidth)
                       .arg(playerResult["username"].toString(), -usernameWidth)
                       .arg(QString::number(playerResult["score"].toInt()), -scoreWidth);

        QGraphicsTextItem *textItem = new QGraphicsTextItem(text);
        textItem->setFont(font);
        textItem->setDefaultTextColor(Qt::red);
        textItem->setZValue(11);

        // Calculate position
        int x = startX + (column * columnWidth);
        int y = startY + (row * itemHeight);
        textItem->setPos(x, y);

        scene->addItem(textItem);

        row++;
        if (row >= maxItemsPerColumn)
        {
            row = 0; // Reset row
            column++; // Move to the next column
        }
    }

    // Set the "GAME OVER!!" text
    // Set the "GAME OVER!!" text
    endText->setPlainText("GAME OVER!!");
    endText->setFont(QFont("Georgia", 48, QFont::Bold));
    endText->setDefaultTextColor(Qt::red); // Make the text red
    endText->setPos(-endText->boundingRect().width() / 2, -scene->height() / 2 + 50); // Shifted up
    endText->setZValue(13);

    overlay->setBrush(QColor(0, 0, 0, 80));  // Semi-transparent black
    activeGameState = false;
    qDebug() << "Game Over";
}


void GraphicsDialog::drawScoreDisplay()
{
    // adds SCORE header and display in top left corner
    // outside scene perimeter
    QFont scoreFont("Georgia",18, QFont::Bold);

    // show score header
    header = new QGraphicsTextItem("SCORE");
    header->setDefaultTextColor(Qt::yellow);
    header->setFont(scoreFont);
    header->setPos(-260, -286);
    // adds the header to scene
    scene->addItem(header);

    // show score display
    // positions the Score variable just under the position of the word SCORE
    display = new QGraphicsTextItem(QString::number(score));
    display->setDefaultTextColor(Qt::yellow);
    display->setFont(scoreFont);
    display->setPos(-155, -286);
    scene->addItem(display);

    // initialize end text
    overlay = new QGraphicsRectItem(-SCENE_WIDTH / 2, -SCENE_HEIGHT / 2, SCENE_WIDTH, SCENE_HEIGHT);
    overlay->setZValue(10);
    scene->addItem(overlay);

    endText = new QGraphicsTextItem;
    endText->setDefaultTextColor(Qt::white);
    endText->setFont(QFont("Georgia", 36, QFont::Bold));
    endText->setDefaultTextColor(Qt::red);
    endText->setPos(-170, -170);
    endText->setZValue(11);
    scene->addItem(endText);
}

void GraphicsDialog::closeEvent(QCloseEvent *event)
{
    emit requestClose();
    event->accept();
}

void GraphicsDialog::setPlayerState(QJsonObject clientData)
{
    int clientId = clientData["clientId"].toInt();
    if (!clientPlayers.contains(clientId)) { return; }
    clientPlayers[clientId]->clientId = clientId;
    clientPlayers[clientId]->username = clientData["username"].toString();
    clientPlayers[clientId]->dead = !clientData["isAlive"].toBool();
    clientPlayers[clientId]->finished = clientData["finishedLastLevel"].toBool();
    clientPlayers[clientId]->inGame = clientData["isInGame"].toBool();
}

void GraphicsDialog::addActivePlayer(int clientId, QString username, const QColor &color)
{
    if (clientPlayers.contains(clientId)) { return; }

    activePlayer = new Player(clientId, username, color);
    activePlayer->resetPlayerPos();
    scene->addItem(activePlayer);
    clientPlayers[clientId] = activePlayer;

    connect(activePlayer, &Player::positionChanged, this, [this]()
        {
        // The lambda captures 'this' (GraphicsDialog), and calls sendPlayerPosition from Dialog
        Dialog *parentDialog = qobject_cast<Dialog*>(parent());
        if (parentDialog)
        {
            parentDialog->sendPlayerPosition(activePlayer->clientId, activePlayer->x, activePlayer->y, activePlayer->angle);
        }
    });
}

void GraphicsDialog::addPlayer(int clientId, QString username, const QColor &color)
{
    if (clientPlayers.contains(clientId)) { return; }

    Player *player = new Player(clientId, username, color);
    player->resetPlayerPos();
    scene->addItem(player);
    clientPlayers[clientId] = player;
}

void GraphicsDialog::removePlayer(int clientId)
{
    if (clientPlayers.contains(clientId))
    {
        removePlayerFromScene(clientId);
        delete clientPlayers[clientId];
        clientPlayers.remove(clientId);
    }
}

void GraphicsDialog::removePlayerFromScene(int clientId)
{
    if (clientPlayers.contains(clientId))
    {
        if (clientPlayers[clientId]->scene() == scene)
        {
            clientPlayers[clientId]->resetPlayerPos();
            scene->removeItem(clientPlayers[clientId]);
        }
    }
}
