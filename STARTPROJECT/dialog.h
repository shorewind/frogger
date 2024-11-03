#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QKeyEvent>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QtGlobal>
#include <QtDebug>
#include "player.h"
#include "defs.h"


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

    QGraphicsScene* scene = nullptr;

    Player* player = nullptr;


    QGraphicsTextItem *header,*display;     // Score header and display object ptrs
    int score = 0;                          // Current score


    // support functions
    void drawScoreDisplay();
    void spawnPlayer();


private slots:

    // Executes in response to player-puck collision
//    void puckCollision();
};

#endif // DIALOG_H
