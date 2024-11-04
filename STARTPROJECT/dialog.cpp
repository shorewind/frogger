#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

        // Allocate and configure scene
        scene = new QGraphicsScene(-SCENE_WIDTH / 2, -SCENE_HEIGHT / 2, SCENE_WIDTH, SCENE_HEIGHT);
        scene->setItemIndexMethod(QGraphicsScene::NoIndex);
        scene->setBackgroundBrush(Qt::black);

        QPixmap background(":/images/background.png");
        background = background.scaled(SCENE_WIDTH, SCENE_HEIGHT, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QGraphicsPixmapItem* backgroundItem = new QGraphicsPixmapItem(background);
        backgroundItem->setPos(-SCENE_WIDTH / 2, -SCENE_HEIGHT / 2);
        backgroundItem->setZValue(-1);
        scene->addItem(backgroundItem);

        // Set the scene for graphicsView and resize it
        ui->graphicsView->setScene(scene);
        ui->graphicsView->setRenderHint(QPainter::Antialiasing);

        // Set fixed size to fit the scene dimensions
        ui->graphicsView->setFixedSize(SCENE_WIDTH, SCENE_HEIGHT);
        this->setFixedSize(SCENE_WIDTH, SCENE_HEIGHT);

        // Add player, timer, and other game elements here
        spawnPlayer();

        QTimer* timer = new QTimer;
        timer->setInterval(24);
        connect(timer, &QTimer::timeout, scene, &QGraphicsScene::advance);
        timer->start();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::keyPressEvent(QKeyEvent *e)
{
    if (e)
    {
        int counter = 0;
        switch (e->key())
        {
            case Qt::Key_A:
            case Qt::Key_J:
            case Qt::Key_Left:
                                    player->goLeft();
            //player->stop();
                                    break;

            case Qt::Key_D:
            case Qt::Key_L:
            case Qt::Key_Right:     player->goRight();
                                    break;

            case Qt::Key_W:
            case Qt::Key_I:
            case Qt::Key_Up:        player->goUp();
                                    break;

            case Qt::Key_S:
            case Qt::Key_K:
            case Qt::Key_Down:      player->goDown();
                                    break;

            // used if player shoots a obstacle
//            case Qt::Key_Space:     //qDebug() << "bang";
//                                    break;

            default:                player->stop();
                                    break;
        }
    }

    //
    QDialog::keyPressEvent(e);
}

void Dialog::spawnPlayer()
{

    // created new datatypes with other .h files
    // Spawn custom Player object in center of scene
    player = new Player;
    // set position of player
    player->setPos(0, 0);
    // adds player to scene
    scene->addItem(player);
}
