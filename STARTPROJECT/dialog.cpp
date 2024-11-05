#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    // Allocate and configure scene
    scene = new QGraphicsScene(-SCENE_WIDTH / 2, -SCENE_HEIGHT / 2, SCENE_WIDTH, SCENE_HEIGHT);
    scene->setBackgroundBrush(Qt::black);

    QPixmap background(":/images/background.png");
    background = background.scaled(SCENE_WIDTH, SCENE_HEIGHT, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QGraphicsPixmapItem* backgroundItem = new QGraphicsPixmapItem(background);
    backgroundItem->setPos(-SCENE_WIDTH / 2, -SCENE_HEIGHT / 2);
    backgroundItem->setZValue(-1);
    scene->addItem(backgroundItem);

    // Set the scene for graphicsView and configure size
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setFixedSize(SCENE_WIDTH, SCENE_HEIGHT);
    this->setFixedSize(SCENE_WIDTH, SCENE_HEIGHT);

    // Add player to the scene
    spawnPlayer();

    // Create and add the log as an Obstacle object
    /*log1 = new Obstacle(50, 20, -SCENE_WIDTH / 2, -50, 5); // Width 50, height 20, starting position, and speed
    log2 = new Obstacle(50, 20, -SCENE_WIDTH / 2 - 200, -50, 5); // Second log, slightly offset
    scene->addItem(log1);
    scene->addItem(log2);
    log1->startMoving();
    log2->startMoving();*/

    // Replace the log creation section with this:

        // Create and add cars as obstacles
        // First row: Chargers moving right
        log1 = new Obstacle(Obstacle::Charger, -SCENE_WIDTH / 2, -50, 5);
        log2 = new Obstacle(Obstacle::Charger, -SCENE_WIDTH / 2 - 200, -50, 5);

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

        // Start moving all cars
        log1->startMoving();
        log2->startMoving();
        supra1->startMoving();
        supra2->startMoving();
        skyline1->startMoving();
        skyline2->startMoving();

}

void Dialog::spawnPlayer()
{
    player = new Player(SCENE_WIDTH, SCENE_HEIGHT); // Pass scene dimensions to player
    player->setPos(0, SCENE_HEIGHT / 2 - 50); // Set starting position
    scene->addItem(player); // Add player to the scene
}


Dialog::~Dialog()
{
    delete ui;
}

void Dialog::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
        case Qt::Key_A:
        case Qt::Key_Left:  player->goLeft(); break;
        case Qt::Key_D:
        case Qt::Key_Right: player->goRight(); break;
        case Qt::Key_W:
        case Qt::Key_Up:    player->goUp(); break;
        case Qt::Key_S:
        case Qt::Key_Down:  player->goDown(); break;
        default: player->stop(); break;
    }

    QDialog::keyPressEvent(e);
}
