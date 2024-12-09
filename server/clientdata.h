#ifndef CLIENTDATA_H
#define CLIENTDATA_H

#include <QString>

struct ClientData
{
    int clientId;
    QString username;
    QString color;
    bool isInGame;
    bool isAlive;
    bool finishedLastLevel;
    bool isReady;
    int levelsPlayed;
    int score;

    ClientData(int id = -1,
               const QString &user = "",
               const QString &col = "white",
               bool inGame = false,
               bool isAlive = false,
               bool finished = false,
               bool ready = false,
               int level = 1,
               int score = 0)  :   clientId(id),
                                   username(user),
                                   color(col),
                                   isInGame(inGame),
                                   isAlive(isAlive),
                                   finishedLastLevel(finished),
                                   isReady(ready),
                                   levelsPlayed(level),
                                   score(score)
                                   {}
};

#endif // CLIENTDATA_H
