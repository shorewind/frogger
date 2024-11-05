#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QKeyEvent>
#include "player.h" // Include the Player class
#include "obstacles.h" // Include the Obstacle class

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

protected:
    void keyPressEvent(QKeyEvent *e);

private:
    Ui::Dialog *ui;
    QGraphicsScene *scene;

   /* Player *player; // Player instance
    Obstacle *log1;  // Moving log obstacle
    Obstacle *log2;  // Moving log obstacle */

    Player *player;
        Obstacle *log1;  // Now a Charger
        Obstacle *log2;  // Now a Charger
        Obstacle *supra1;
        Obstacle *supra2;
        Obstacle *skyline1;
        Obstacle *skyline2;

    // Constants for scene dimensions
    static constexpr int SCENE_WIDTH = 800;   // Width of the game scene
    static constexpr int SCENE_HEIGHT = 600;  // Height of the game scene

    // Support functions
    void spawnPlayer();
};

#endif // DIALOG_H
