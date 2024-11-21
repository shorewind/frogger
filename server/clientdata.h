#ifndef CLIENTDATA_H
#define CLIENTDATA_H

#include <QString>

struct ClientData
{
    int clientId;
    QString username;
    QString color;
    bool isInGame;

    ClientData(int id = -1, const QString &user = "", const QString &col = "white", bool inGame = false)
        : clientId(id), username(user), color(col), isInGame(inGame) {}
};

#endif // CLIENTDATA_H
