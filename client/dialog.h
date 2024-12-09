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
#include <QCheckBox>

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
    void sendPlayerPosition(int clientId, int x, int y, int angle);

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
    QMap<int, QString> clientColors;
    QJsonArray clientDataArray;
    QJsonObject clientData;
    QStandardItemModel *leaderboardModel, *historyModel, *userHistoryModel;

    void showAllSessions();
    void showSessionsForUser();
    void setGameData(QJsonObject& gameDataObj);

    void disconnectFromServer();
    QString getLocalIpAddress();
    void setLocalIpAddress();
    void setPlayerColor(QString &color);
    int parseClientIdFromMsg(const QString &msg);
    bool isPlayerAlive(int clientId);
    bool isUsernameTaken(QString username);

private slots:
    void connectToServer();
    void leaveGame();
    void processMsg();
    void sendMsg();

    void onColorButtonClick();
    void onSubmitButtonClick();
    void onSendButtonClick();
    void submitUsername();
    void setPlayerReady();
};

#endif // DIALOG_H
