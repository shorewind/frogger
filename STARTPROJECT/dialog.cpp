#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    // Allocate and configure scene
    // makes the center of the scene 0,0
    scene = new QGraphicsScene(-SCENE_WIDTH/2, -SCENE_HEIGHT/2,
                               SCENE_WIDTH, SCENE_HEIGHT);
    // always turn on NoIndex and Antialiasing
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setBackgroundBrush(Qt::black);

    // Make scene object the data source for the view object
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);


    // Add score header and display text items
//    drawScoreDisplay();

    // Create a player object and add it to the scene object
    spawnPlayer();


    // Configure timer object to drive animation
    QTimer* timer = new QTimer;
    timer->setInterval(24);

    // advance function, everytime the timer went off, run the dialog class advance method.
    // tells the scene object for all ofthe items in the scene run their personal advance method
    connect(timer, &QTimer::timeout, scene, &QGraphicsScene::advance);
    // need to start QTimer object
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
        //int counter = 0;
        switch (e->key())
        {
            case Qt::Key_A:
            case Qt::Key_J:
            case Qt::Key_Right:
                                    player->goLeft();
            //player->stop();
                                    break;

            case Qt::Key_D:
            case Qt::Key_L:
            case Qt::Key_Left:
                                    player->goRight();
                                    break;

            case Qt::Key_W:
            case Qt::Key_I:
            case Qt::Key_Down:
                                    player->goUp();
                                    break;

            case Qt::Key_S:
            case Qt::Key_K:
            case Qt::Key_Up:
                                    player->goDown();
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
