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
#include <QJsonArray>
#include <QTableView>
#include <QStandardItemModel>

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
    bool isPlayerAlive(int clientId);

public slots:
    void sendJson(QJsonObject data);
    void sendPlayerPosition(int clientId, int x, int y);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::Dialog *ui;

    QUdpSocket* socket;
    bool validConnection = false;
    GraphicsDialog *graphicsDialog = nullptr;
    int activeClientId;
    QString ip;
    quint16 port;
    QSet<int> activeClients;
    QString playerColor;
//    QList<QString> usedColors;
    QMap<int, QString> clientColors;
    QJsonArray clientDataArray;
    QJsonObject clientData;
    QStandardItemModel *qmLeaderboard, *qmHistory, *qmUserHistory;

    void showAllSessions();
    void showSessionsForUser();
    void setGameData(QJsonObject& gameDataObj);

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
