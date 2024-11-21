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
#include <QNetworkInterface>

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
    QString playerUsername;

public slots:
    void sendJson(QJsonObject data);
    void sendPlayerPosition(int clientId, qreal x, qreal y);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::Dialog *ui;

    QUdpSocket* socket;
    GraphicsDialog *graphicsDialog = nullptr;
    int activeClientId;
    QString ip;
    quint16 port;
    QSet<int> activeClients;
    QString playerColor;
//    QList<QString> usedColors;
    QMap<int, QString> clientColors;

private slots:
    void connectToServer();
    void disconnectFromServer();
    void leaveGame();
    void processMsg();
    void sendMsg();
    QString getLocalIpAddress();
    void setLocalIpAddress();
    int parseClientIdFromMsg(const QString &msg);
    void onColorButtonClick();
    void onSubmitButtonClick();
    void onSendButtonClick();
    void submitUsername();
    void setPlayerColor(QString &color);
};

#endif // DIALOG_H
