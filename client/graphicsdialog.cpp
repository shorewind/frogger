#include "graphicsdialog.h"
#include "defs.h"
#include "dialog.h"

GraphicsDialog::GraphicsDialog(QWidget *parent, QUdpSocket *socket) :
    QDialog(parent),
    scene(new QGraphicsScene(this)),
    activePlayer(nullptr),
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
}

GraphicsDialog::~GraphicsDialog() {
    delete scene; // Clean up the scene
}

void GraphicsDialog::keyPressEvent(QKeyEvent *e)
{
    if (!activePlayer) {
        return;  // If no active player, just return (do nothing)
    }

    qDebug() << activePlayer->clientId;

    switch (e->key())
    {
        case Qt::Key_A: activePlayer->goLeft(); break;
        case Qt::Key_D: activePlayer->goRight(); break;
        case Qt::Key_W: activePlayer->goUp(); break;
        case Qt::Key_S: activePlayer->goDown(); break;
        default: activePlayer->stop(); break;
    }
    QDialog::keyPressEvent(e);
}

//void GraphicsDialog::sendPlayerPosition() {
//    qDebug() << "sending player position";

//    QJsonObject message;
//    message["type"] = "POSITION";
//    QJsonArray playerPosArray;

//    QJsonObject playerPosData;
//    playerPosData["clientId"] = activePlayer->clientId;
//    playerPosData["x"] = activePlayer->x;
//    playerPosData["y"] = activePlayer->x;

//    playerPosArray.append(playerPosData);
//    message["players"] = playerPosArray;

//    QJsonDocument doc(message);
//    QByteArray data = doc.toJson();
//    qDebug() << message;
//    if (socket->state() != QAbstractSocket::BoundState) {
//        qDebug() << "Socket is not bound!";

//    }

//    if (!socket->writeDatagram(data, QHostAddress(serverIp), serverPort))
//    {
//        qDebug() << "Error sending datagram: " << socket->errorString();
//    }
//}

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

        // Update position if the player exists
        if (clientPlayers.contains(clientId)) {
            clientPlayers[clientId]->setPos(x, y);
            qDebug() << "client Id " << clientId << " " << x << ", " << y;
        }
    }
}

void GraphicsDialog::closeEvent(QCloseEvent *event) {
    emit requestClose();
    event->accept();
}

void GraphicsDialog::addActivePlayer(int clientId, const QColor &color) {
    if (clientPlayers.contains(clientId)) { return; }

    activePlayer = new Player(clientId, color);
    activePlayer->setPos(clientId * 40, 40); // Adjust position as needed
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
    player->setPos(clientId * 40, 40); // Adjust position as needed
    scene->addItem(player);
    clientPlayers[clientId] = player;

    qDebug() << "added player " << clientId;
}

void GraphicsDialog::removePlayer(int clientId) {
    if (clientPlayers.contains(clientId)) {
        scene->removeItem(clientPlayers[clientId]);
        delete clientPlayers[clientId];
        clientPlayers.remove(clientId);
    }
}
