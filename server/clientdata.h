#ifndef CLIENTDATA_H
#define CLIENTDATA_H

#include <QString>

struct ClientData {
    int clientId;      // Unique client ID
    QString username;  // Client's username
    QString color;     // Client's selected color
    bool isInGame;     // Whether the client is in the game

    // Constructor with default values
    ClientData(int id = -1, const QString &user = "", const QString &col = "", bool inGame = false)
        : clientId(id), username(user), color(col), isInGame(inGame) {}
};

#endif // CLIENTDATA_H
