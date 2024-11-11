#ifndef CLIENT_H
#define CLIENT_H

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
#include <QLineEdit>
#include <QTextBrowser>

class Dialog;
class GraphicsDialog;

class Client : public QDialog
{
    Q_OBJECT

public:
    explicit Client(Dialog *parent = nullptr);

public slots:
    void sendJson(QJsonObject data);
    void sendPlayerPosition(int clientId, qreal x, qreal y);
    void connectToServer();
    void closeEvent(QCloseEvent *event) override;

private:
    Dialog* mainDialog;

    QUdpSocket* socket;
    GraphicsDialog *graphicsDialog;
    int activeClientId;
    QString ip;
    quint16 port;
    QSet<int> activeClients;

private slots:
    void disconnectFromServer();
    void processMsg();
    void sendMsg();
    QString getLocalIpAddress();
    void setLocalIpAddress();

    int parseClientIdFromMsg(const QString &msg);
    QColor generateColorForClient(int clientId);
};

#endif // CLIENT_H
