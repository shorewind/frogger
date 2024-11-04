#include "graphicsdialog.h"
#include "defs.h"

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
}

GraphicsDialog::~GraphicsDialog() {
    delete scene; // Clean up the scene
}

void GraphicsDialog::keyPressEvent(QKeyEvent *e)
{
//    qDebug() << player;
    switch (e->key())
    {
        case Qt::Key_A: player->goLeft(); break;
        case Qt::Key_D: player->goRight(); break;
        case Qt::Key_W: player->goUp(); break;
        case Qt::Key_S: player->goDown(); break;
        default: player->stop(); break;
    }
    QDialog::keyPressEvent(e);
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
