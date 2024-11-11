#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QUdpSocket>
#include <QHostAddress>
#include <QtDebug>
#include <QCloseEvent>
#include <QColor>
#include <QRegExp>
#include <QList>
#include <QJsonObject>
#include <QJsonDocument>



class GraphicsDialog;

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

public slots:
    void sendJson(QJsonObject data);
    void sendPlayerPosition(int clientId, qreal x, qreal y);
    void sendObstaclePosition(int obstacleId, int obstacleType, int x, int y, int speed);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::Dialog *ui;

    QUdpSocket* socket;
    GraphicsDialog *graphicsDialog;
    int activeClientId;
    QString ip;
    quint16 port;
    QSet<int> activeClients;






private slots:
    void connectToServer();
    void disconnectFromServer();
    void processMsg();
    void sendMsg();


    int parseClientIdFromMsg(const QString &msg);
    QColor generateColorForClient(int clientId);
};

#endif // DIALOG_H
