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

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::Dialog *ui;

    QUdpSocket* socket;
    GraphicsDialog *graphicsDialog;
    QString ip;
    quint16 port;

private slots:
    void connectToServer();
    void disconnectFromServer();
    void processMsg();
    void sendMsg();

    void sendJson(QJsonObject data);

    int parseClientIdFromMsg(const QString &msg);
    QColor generateColorForClient(int clientId);
//    void sendPlayerPosition(int clientId, qreal x, qreal y);
};

#endif // DIALOG_H
