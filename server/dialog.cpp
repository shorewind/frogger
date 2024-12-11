#include "dialog.h"
#include "ui_dialog.h"
#include "defs.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    setLocalIpAddress();
    ui->portEdit->setText(QString::number(DEFAULT_PORT));
    ui->textBrowser->append("Press the configure button to start a game server.");

    connect(ui->configureButton, &QPushButton::clicked, this, &Dialog::configureServer);
    connect(ui->startButton, &QPushButton::clicked, this, &Dialog::startGame);
    connect(ui->button1, &QPushButton::clicked, this, &Dialog::kickClient);
    connect(ui->button2, &QPushButton::clicked, this, &Dialog::kickClient);
    connect(ui->button3, &QPushButton::clicked, this, &Dialog::kickClient);
    connect(ui->button4, &QPushButton::clicked, this, &Dialog::kickClient);

    for (int i = 1; i <= MAX_CLIENTS; ++i)
    {
        availableIds.append(i);
    }

    setupDatabase();

    // set up carbon fiber texture, making it red and black
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Background, QBrush(QPixmap(":/images/redb.jpg").scaled(this->size(), Qt::KeepAspectRatioByExpanding)));
    this->setPalette(palette);

    // custom font from the resource file
    int fontId = QFontDatabase::addApplicationFont(":/images/Orbitron-VariableFont_wght.ttf");
    if (fontId != -1)
    {
        QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QFont orbitronFont(fontFamily, 24, QFont::Bold);

        // apply to the title label
        ui->label->setFont(orbitronFont);
        ui->label->setStyleSheet(R"(
            QLabel {
                color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 #FF4500, stop:1 #FF6347);
                text-align: center;
            }
        )");
    }
    else
    {
        qDebug() << "Failed to load Orbitron font!";
    }

    // making the tabwidget coooleerrrr
    ui->tabWidget->setStyleSheet(R"(
       QTabWidget::pane {
           border: none;
       }

       QTabBar::tab {
           background-color: #333333;
           color: white;
           padding: 10px;
           border-radius: 5px;
           font: bold 14px 'Orbitron', sans-serif;
           min-width: 120px; /* Adjust this value to make the tab wider */
       }

       QTabBar::tab:selected {
           background-color: #FF4500; /* Neon red */
           color: black;
       }

       QTabBar::tab:hover {
           background-color: #FF6347; /* lighter Neon red */
       }

       QTabWidget::tab-bar {
           alignment: center;
       }
    )");

    // making the manage label look cool
    ui->label_2->setStyleSheet(R"(
              QLabel {
                 background-color: #000000;
                 color: #FF4500;
                 border: 2px solid #FF4500;
                 font: bold 14px 'Orbitron', sans-serif;
               }
         )");


    // making the text browser look cool
    ui->textBrowser->setStyleSheet(R"(
             QTextBrowser {
                background-color: #000000;
                color: #FF4500;
                border: 2px solid #FF4500;
                font-family: "Courier New";
                font-size: 12px;
              }
        )");

   // styling for IP and Port input fields
   QString inputFieldStyle = R"(
        QLineEdit {
               border: 2px solid #FF4500;
               border-radius: 5px;
               padding: 5px;
               background-color: #333333;
               color: white;
           }
         QLineEdit:focus {
               border-color: #00ffff;
           }
       )";
    ui->ipEdit->setStyleSheet(inputFieldStyle);
    ui->portEdit->setStyleSheet(inputFieldStyle);

    // styling for the configure button
    ui->configureButton->setStyleSheet(R"(
        QPushButton {
            background-color: #222222;
            color: white;
            border: 2px solid #00ffff;
            border-radius: 10px;
            font: bold 14px;
        }
        QPushButton:hover {
            background-color: #00ffff;
            color: #000000;
        }
    )");

    // styling for the start button
    ui->startButton->setStyleSheet(R"(
        QPushButton {
            background-color: #222222;
            color: white;
            border: 2px solid #00ffff;
            border-radius: 10px;
            font: bold 14px;
        }
        QPushButton:hover {
            background-color: #00ffff;
            color: #000000;
        }
    )");
}

void Dialog::kickClient()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    int clientId = -1;
    if (button) {
        if (button == ui->button1) {
            clientId = 1;
        } else if (button == ui->button2) {
            clientId = 2;
        } else if (button == ui->button3) {
            clientId = 3;
        } else if (button == ui->button4) {
            clientId = 4;
        }
    }

    QString clientKey = getClientKeyFromClientId(clientId);

    if (clientKey != "")
    {
        QStringList parts = clientKey.split(":");
        if (parts.size() == 2)
        {
            QHostAddress address(parts[0]);
            quint16 port = parts[1].toUInt();
            QJsonObject kickedMsg;
            kickedMsg["type"] = "KICKED";
            kickedMsg["message"] = "You got kicked!";

            QJsonDocument kickedJson(kickedMsg);

            ui->textBrowser->append(QString("%1 got kicked!").arg(clientIdMap[clientKey].username));
            socket->writeDatagram(kickedJson.toJson(), address, port);
        }
        removeClient(clientKey);
        broadcastActiveClients();
    }
}

QString Dialog::getClientKeyFromClientId(int clientId)
{
    for (auto it = clientIdMap.begin(); it != clientIdMap.end(); ++it)
    {
        if (it.value().clientId == clientId)
        {
            return it.key();
        }
    }
    return "";
}

void Dialog::setupDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");  // creates a DB connection
    db.setDatabaseName("froggy_game_data.db");
    db.open();
    QSqlQuery query;

    // for clearing tables
//    query.exec("DROP TABLE games");
//    query.exec("DROP TABLE sessions");
    bool createdGamesTable = query.exec(
        "CREATE TABLE IF NOT EXISTS games ("
        "id INTEGER,"
        "timestamp TIMESTAMP DEFAULT (datetime('now', 'localtime')),"
        "winner_username TEXT NOT NULL,"
        "high_score INTEGER NOT NULL,"
        "max_level INTEGER NOT NULL"
        ");"
    );
    if (!createdGamesTable)
        qDebug () << "Error Creating Games Table: " << query.lastError().text();

    bool createdSessionsTable = query.exec(
        "CREATE TABLE IF NOT EXISTS sessions ("
        "id INTEGER PRIMARY KEY,"
        "game_id INTEGER NOT NULL,"
        "player_username TEXT NOT NULL,"
        "score INTEGER NOT NULL,"
        "levels_played INTEGER NOT NULL"
        ");"
    );
    if (!createdSessionsTable)
        qDebug () << "Error Creating Sessions Table: " << query.lastError().text();

    query.exec("SELECT MAX(game_id) FROM sessions");
    if (query.next()) {
        currentGameId = query.value(0).toInt();
    }
    else
    {
        currentGameId = 0;
    }
}

bool sessionExists(int gameId, const QString &playerUsername)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM sessions WHERE game_id = :game_id AND player_username = :player_username");
    query.bindValue(":game_id", gameId);
    query.bindValue(":player_username", playerUsername);
    query.exec();

    if (query.next())
    {
        return query.value(0).toInt() > 0;
    }
    return false;
}

void insertOrUpdateSession(int gameId, const QString &playerUsername, int score, int levelsPlayed)
{
    QSqlQuery query;

    if (score == 0) return;

    if (sessionExists(gameId, playerUsername))
    {
        query.prepare("UPDATE sessions SET score = :score, levels_played = :levels_played "
                      "WHERE game_id = :game_id AND player_username = :player_username");
        query.bindValue(":score", score);
        query.bindValue(":levels_played", levelsPlayed);
        query.bindValue(":game_id", gameId);
        query.bindValue(":player_username", playerUsername);

        if (!query.exec())
        {
            qDebug() << "Error updating session: " << query.lastError().text();
            return;
        }
    }
    else
    {
        query.prepare("INSERT INTO sessions (game_id, player_username, score, levels_played) "
                      "VALUES (:game_id, :player_username, :score, :levels_played)");
        query.bindValue(":game_id", gameId);
        query.bindValue(":player_username", playerUsername);
        query.bindValue(":score", score);
        query.bindValue(":levels_played", levelsPlayed);

        if (!query.exec())
        {
            qDebug() << "Error inserting session: " << query.lastError().text();
            return;
        }
    }
    return;
}

void Dialog::configureServer()
{
    bool ipHasText = !ui->ipEdit->text().isEmpty();
    bool portHasText = !ui->portEdit->text().isEmpty();

    if (!ipHasText || !portHasText) { return; }

    if (availableIds.size() != MAX_CLIENTS)
    {
        qDebug() << availableIds.size() << MAX_CLIENTS;
        QJsonObject disconnectAllMessage;
        disconnectAllMessage["type"] = "DISCONNECT_ALL";
        disconnectAllMessage["message"] = "Server is shutting down. Disconnecting all clients.";

        tx(disconnectAllMessage);
    }

    if (socket && socket->isOpen()) {
        qDebug() << "closing socket";
        socket->close();
        qDebug() << "Server socket disconnected.";
    }

    socket = new QUdpSocket(this);

    QString ip = ui->ipEdit->text();
    quint16 port = ui->portEdit->text().toUInt();

    // bind the socket to the specified IP and port
    bool success = socket->bind(QHostAddress(ip), port);
    if (!success)
    {
        ui->textBrowser->append("Failed to bind server socket: " + socket->errorString());
        return;
    }

    connect(socket, &QUdpSocket::readyRead, this, &Dialog::rx);

    ui->ipEdit->clear();
    ui->portEdit->clear();
    ui->textBrowser->clear();
    clientIdMap.clear();

    if (socket->isValid())
    {
        ui->textBrowser->append("Server Active: " + ip + ":" + QString::number(port));
        ui->textBrowser->append("Connect at least one client to start game.");
//        ui->configureButton->setEnabled(false);
    }

    activeGame = false;
}

void Dialog::closeEvent(QCloseEvent *event)
{
    if (availableIds.size() != MAX_CLIENTS)
    {
        qDebug() << availableIds.size() << MAX_CLIENTS;
        QJsonObject disconnectAllMessage;
        disconnectAllMessage["type"] = "DISCONNECT_ALL";
        disconnectAllMessage["message"] = "Server is shutting down. Disconnecting all clients.";

        tx(disconnectAllMessage);
    }

    if (socket && socket->isOpen()) {
        qDebug() << "closing socket";
        socket->close();
        qDebug() << "Server socket disconnected.";
    }

    event->accept();
}

void Dialog::startGame()
{
    if (activeGame) return;
    // break if not all players ready

    QPushButton *button = qobject_cast<QPushButton*>(sender());

    if (!button || button != ui->startButton)
    {
        for (auto it = clientIdMap.begin(); it != clientIdMap.end(); ++it)
        {
            if (it.value().clientId != -1 && it.value().isReady == false)
            {
                return;
            }
        }
    }

    QJsonObject startMessage;
    startMessage["type"] = "START";
    startMessage["message"] = "Game Started.";
    ui->textBrowser->append(startMessage["message"].toString());
    tx(startMessage);

    activeGame = true;
    gameOver = false;
    ui->startButton->setEnabled(false);
    ui->configureButton->setEnabled(false);
    currentGameId++;

    // add all connected clients to the game
    for (auto it = clientIdMap.begin(); it != clientIdMap.end(); ++it)
    {
        if (it.value().clientId != -1)
        {
            it.value().isInGame = true;  // mark client as in the game
            it.value().isAlive = true;
            it.value().isReady = false;
            clientIdsInGame.append(it.value().clientId);
        }
    }

    broadcastActiveClients();
}


void Dialog::rx()
{
    while (socket->hasPendingDatagrams())
    {
        msg = socket->receiveDatagram();

        QString clientIP = msg.senderAddress().toString();
        quint16 senderPort = msg.senderPort();
        QByteArray incomingMsg = msg.data();
        QHostAddress senderAddress = msg.senderAddress();

        QString clientKey = QString("%1:%2").arg(clientIP).arg(senderPort);

        QJsonDocument doc = QJsonDocument::fromJson(incomingMsg);
        if (!doc.isObject()) return;  // check if the received data is a valid JSON object

        QJsonObject jsonObj = doc.object();
        QString type = jsonObj["type"].toString();

        qDebug() << "rx";
        qDebug() << jsonObj;

        if (type == "DISCONNECT")
        {
            removeClient(clientKey);
            broadcastActiveClients();
            return;
        }
        else if (type == "CONNECT")
        {
            // handle new connections
            int clientId = -1;
            if (!clientIdMap.contains(clientKey) && !availableIds.isEmpty())
            {
                clientId = availableIds.takeFirst();
                clientIdMap[clientKey].clientId = clientId;
                clientIdMap[clientKey].isInGame = false;
                clientIdMap[clientKey].username = QString("Client %1").arg(clientId);  // set default username

                clientAddresses.append(senderAddress);
                clientPorts.append(senderPort);
                setActiveClient(clientIdMap[clientKey]);
            }
            else
            {
                // reject the connection if no IDs are available
                QJsonObject rejectionMessage;
                rejectionMessage["type"] = "REJECTION";
                rejectionMessage["message"] = "Server is full. Try again later.";
                QJsonDocument rejectionDoc(rejectionMessage);
                socket->writeDatagram(rejectionDoc.toJson(), senderAddress, senderPort);
                ui->textBrowser->append("Connection rejected: Server is full.");
                return;
            }

            // notify the new client
            QJsonObject welcomeMessage;
            welcomeMessage["type"] = "WELCOME";
            welcomeMessage["message"] = "Welcome to Fast and Froggy! You are Client " + QString::number(clientId);
            QJsonDocument welcomeDoc(welcomeMessage);
            socket->writeDatagram(welcomeDoc.toJson(), senderAddress, senderPort);

            // broadcast new connection
            QJsonObject broadcastMessage;
            broadcastMessage["type"] = "JOINED";
            broadcastMessage["message"] = QString("%1 (%2:%3) has joined the server.")
                    .arg(clientIdMap[clientKey].username)
                    .arg(clientIP)
                    .arg(senderPort);
            ui->textBrowser->append(broadcastMessage["message"].toString());
            tx(broadcastMessage);

            broadcastActiveClients();
            sendGameData();
        }
        else if (type == "LEAVE")
        {
            if (clientIdMap.contains(clientKey) && clientIdMap[clientKey].isInGame == true)
            {
                int clientId = clientIdMap[clientKey].clientId;

                clientIdsInGame.removeAll(clientId);
                clientIdMap[clientKey].isInGame = false;
                clientIdMap[clientKey].isAlive = false;
                clientIdMap[clientKey].finishedLastLevel = false;

                QStringList parts = clientKey.split(':');

                if (parts.size() == 2)
                {
                    QHostAddress address = QHostAddress(parts[0]);
                    quint16 port = static_cast<quint16>(parts[1].toUInt());

                    QJsonObject goodbyeMsg;
                    goodbyeMsg["type"] = "GOODBYE";
                    goodbyeMsg["message"] = "You left the game! You were Client " + QString::number(clientId);
                    QJsonDocument goodbyeDoc(goodbyeMsg);
                    socket->writeDatagram(goodbyeDoc.toJson(), senderAddress, senderPort);


                    QJsonObject leaveMsg;
                    leaveMsg["type"] = "MESSAGE";
                    leaveMsg["message"] = QString("%1 (%2:%3) has left the game.")
                                                     .arg(clientIdMap[clientKey].username)
                                                     .arg(address.toString())
                                                     .arg(port);

                    ui->textBrowser->append(leaveMsg["message"].toString());
                    tx(leaveMsg);
                }

                if (clientIdsInGame.isEmpty())
                {
                    activeGame = false;
                    ui->startButton->setEnabled(true);
                    ui->configureButton->setEnabled(false);
                    ui->textBrowser->append("Game Closed.");

                    QJsonObject gameClosedMessage;
                    gameClosedMessage["type"] = "GAME_CLOSED";
                    gameClosedMessage["message"] = "Game Closed.";
                    tx(gameClosedMessage);
                }
            }
            broadcastActiveClients();
            checkGameState();
            insertOrUpdateSession(currentGameId, clientIdMap[clientKey].username, clientIdMap[clientKey].score, clientIdMap[clientKey].levelsPlayed);
            sendGameData();
            return;
        }
        else if (type == "READY")
        {
            if (jsonObj["status"].toBool() == true)
            {
                clientIdMap[clientKey].isReady = true;
                startGame();
            }
            else if (jsonObj["status"].toBool() == false)
            {
                clientIdMap[clientKey].isReady = false;
            }
            ui->textBrowser->append(jsonObj["message"].toString());

            QJsonObject outgoingMessage;
            outgoingMessage["type"] = "MESSAGE";
            outgoingMessage["message"] = jsonObj["message"];
            tx(outgoingMessage);
        }
        else if (type == "MESSAGE")
        {
            // handle incoming messages from clients
            QString message = jsonObj["message"].toString();

            if (clientIdMap.contains(clientKey) && !message.isEmpty())
            {
                ui->textBrowser->append(QString("%1 (%2:%3): %4")
                                         .arg(clientIdMap[clientKey].username)
                                         .arg(clientIP)
                                         .arg(senderPort)
                                         .arg(message));

                QJsonObject outgoingMessage;
                outgoingMessage["type"] = "MESSAGE";
                outgoingMessage["message"] = QString("%1 (%2:%3)> %4")
                        .arg(clientIdMap[clientKey].username)
                        .arg(clientIP)
                        .arg(senderPort)
                        .arg(message);
                tx(outgoingMessage);
            }
        }
        else if (type == "POSITION")
        {
            // handle position updates from the client
            playersArray = jsonObj["players"].toArray();

            updatePlayerPositions(playersArray);
            broadcastPlayerPositions();
        }
        else if (type == "USERNAME")
        {
            QString username = jsonObj["username"].toString();
            if (clientIdMap.contains(clientKey))
            {
                clientIdMap[clientKey].username = username;
                ui->textBrowser->append(QString("Client %1 set their username to: %2")
                                         .arg(clientIdMap[clientKey].clientId)
                                         .arg(username));
            }
            broadcastActiveClients();
            sendGameData();
        }
        else if (type == "PLAYER_COLOR")
        {
            QString color = jsonObj["color"].toString();
            if (clientIdMap.contains(clientKey))
            {
                clientIdMap[clientKey].color = color;
                ui->textBrowser->append(QString("%1 set their color to: %2")
                                         .arg(clientIdMap[clientKey].username)
                                         .arg(color));
            }
            broadcastActiveClients();
        }
        else if (type == "SCORE")
        {
            if (clientIdMap.contains(clientKey))
            {
                clientIdMap[clientKey].isAlive = jsonObj["isAlive"].toBool();
                clientIdMap[clientKey].finishedLastLevel = jsonObj["finishedLastLevel"].toBool();
                clientIdMap[clientKey].levelsPlayed = jsonObj["levelsPlayed"].toInt();
                clientIdMap[clientKey].score = jsonObj["score"].toInt();

                if (!clientIdMap[clientKey].isAlive)
                {
                    QString playerDiedMsg = QString("%1 died.").arg(clientIdMap[clientKey].username);
                    QJsonObject outgoingMessage;
                    outgoingMessage["type"] = "MESSAGE";
                    outgoingMessage["message"] = playerDiedMsg;
                    ui->textBrowser->append(playerDiedMsg);
                    tx(outgoingMessage);
                }
                else if (clientIdMap[clientKey].isAlive && clientIdMap[clientKey].finishedLastLevel)
                {
                    playersFinished.append(clientIdMap[clientKey].clientId);
                    QJsonObject playerFinishedMsg;
                    playerFinishedMsg["type"] = "PLAYER_FINISHED";
                    playerFinishedMsg["clientId"] = clientIdMap[clientKey].clientId;
                    playerFinishedMsg["placement"] = playersFinished.indexOf(clientIdMap[clientKey].clientId) + 1;  // 1-based placement
                    QList<int> pointsForPlacement = {250, 200, 150, 100};
                    int placementPoints = (playerFinishedMsg["placement"].toInt() < pointsForPlacement.size()) ? pointsForPlacement[playerFinishedMsg["placement"].toInt()-1] : 0;
                    clientIdMap[clientKey].score += placementPoints;
                    playerFinishedMsg["score"] = clientIdMap[clientKey].score;
                    playerFinishedMsg["message"] = QString("%1 finished level in rank %2 earning %3 bonus points. Total score: %4")
                                                 .arg(clientIdMap[clientKey].username)
                                                 .arg(playerFinishedMsg["placement"].toInt())
                                                 .arg(placementPoints)
                                                 .arg(playerFinishedMsg["score"].toInt());
                    ui->textBrowser->append(playerFinishedMsg["message"].toString());
                    tx(playerFinishedMsg);
                }
            }
            broadcastActiveClients();
            insertOrUpdateSession(currentGameId, clientIdMap[clientKey].username, clientIdMap[clientKey].score, clientIdMap[clientKey].levelsPlayed);
            checkGameState();
            sendGameData();
        }
    }
}

void Dialog::checkGameState()
{
    qDebug() << "checking game state";

    if (clientIdsInGame.size() == 0) { return; }

    bool allPlayersDead = true;

    if (!clientIdMap.isEmpty())
    {
        for (auto &player : clientIdMap.values())
        {
            if (player.isAlive)  // if a player is not dead, set the flag to false and exit the loop
            {
                allPlayersDead = false;
                break;   // exit early, no need to check the rest of the players
            }
        }
    }

    if (allPlayersDead && !gameOver)
    {
        QJsonArray playerResults;
        qDebug() << "game over";
        gameOver = true;

        QSqlQuery query;
        QString queryString = QString("SELECT player_username, score, levels_played FROM sessions WHERE game_id = '%1' ORDER BY score DESC").arg(currentGameId);
        if (!query.exec(queryString))
        {
            qDebug() << "Error determining winner: " << query.lastError().text();
            return;
        }

        int placement = 1;
        QJsonObject gameOverMsg;
        gameOverMsg["type"] = "GAME_OVER";

        while (query.next())
        {
            QJsonObject playerResult;
            playerResult["username"] = query.value("player_username").toString();
            playerResult["score"] = query.value("score").toInt();
            playerResult["levels_played"] = query.value("levels_played").toInt();
            playerResult["placement"] = placement;
            playerResults.append(playerResult);

            if (placement == 1)
            {
                gameOverMsg["message"] = QString("Game Over!\n%1 won with a score of %2 after %3 level(s).")
                                            .arg(playerResult["username"].toString())
                                            .arg((QString::number(playerResult["score"].toInt())))
                                            .arg(QString::number(playerResult["levels_played"].toInt()));
                ui->textBrowser->append(gameOverMsg["message"].toString());
            }
            placement++;
        }

        gameOverMsg["results"] = playerResults;

        tx(gameOverMsg);
        logGame();
        sendGameData();
        playersFinished.clear();
        return;   // exit early if the game is over
    }

    bool done = true;
    for(auto &player : clientIdMap.values())
    {
        if (player.finishedLastLevel || !player.isAlive || !player.isInGame)  // player is either dead or at the lily pads
        {
            done = true;
        }
        else    // player is either not dead or not to the lily pads yet, keep round running
        {
            done = false;
            break;  // only need one to throw false so exit early
        }
    }

    if (done && !gameOver) // if none of the players are still playing and the round hasn't already ended
    {
        qDebug() << "level over";
        QJsonObject levelOverMsg;
        levelOverMsg["type"] = "LEVEL_OVER";

        levelOverMsg["message"] = "Level Over.";
        ui->textBrowser->append(levelOverMsg["message"].toString());
        tx(levelOverMsg);
        sendGameData();
        playersFinished.clear();

        for (int i = 0; i < clientAddresses.size(); ++i)
        {
            QString clientKey = QString("%1:%2").arg(clientAddresses[i].toString()).arg(clientPorts[i]);
            if (clientIdMap.contains(clientKey))
            {
                clientIdMap[clientKey].finishedLastLevel = false;
            }
        }
    }
}

void Dialog::logGame()
{
    QSqlQuery checkQuery;

    if (!checkQuery.exec(QString("SELECT COUNT(*) FROM games WHERE id = '%1'").arg(currentGameId)))
    {
        qDebug() << "Error checking if game exists: " << checkQuery.lastError().text();
        return;
    }

    checkQuery.next();
    int gameCount = checkQuery.value(0).toInt();

    if (gameCount > 0)
    {
        qDebug() << "Game result for game_id: " << currentGameId << " already exists. Skipping insertion.";
        return;
    }

    QSqlQuery query;
    QString queryString = QString("SELECT player_username, score, levels_played FROM sessions WHERE game_id = '%1' ORDER BY score DESC LIMIT 1").arg(currentGameId);
    if (!query.exec(queryString))
    {
        qDebug() << "Error determining winner: " << query.lastError().text();
        return;
    }

    if (query.next())
    {
        QString winnerUsername = query.value("player_username").toString();
        int highScore = query.value("score").toInt();
        int maxLevel = query.value("levels_played").toInt();

        QSqlQuery insertQuery;
        insertQuery.prepare("INSERT INTO games (id, timestamp, winner_username, high_score, max_level) "
                            "VALUES (:id, datetime('now', 'localtime'), :winner_username, :high_score, :max_level)");
        insertQuery.bindValue(":id", currentGameId);
        insertQuery.bindValue(":winner_username", winnerUsername);
        insertQuery.bindValue(":high_score", highScore);
        insertQuery.bindValue(":max_level", maxLevel);

        if (!insertQuery.exec())
        {
            qDebug() << "Error inserting game result: " << insertQuery.lastError().text();
        }
        else
        {
            qDebug() << "Game result inserted successfully.";
        }
    }
    else
    {
        qDebug() << "No sessions found for game_id: " << currentGameId;
    }
}

void Dialog::sendGameData()
{
    QSqlQuery query;

    query.exec("SELECT * FROM games ORDER BY timestamp DESC");
    QJsonArray gamesArray;

    while (query.next())
    {
        QJsonObject gameObject;
        gameObject["id"] = query.value("id").toInt();
        gameObject["timestamp"] = query.value("timestamp").toString();
        gameObject["winner_username"] = query.value("winner_username").toString();
        gameObject["high_score"] = query.value("high_score").toInt();
        gameObject["max_level"] = query.value("max_level").toInt();

        gamesArray.append(gameObject);
    }

    query.exec(R"(
                   SELECT sessions.id, sessions.game_id, sessions.player_username, sessions.score,
                          sessions.levels_played, games.timestamp,
                          (sessions.player_username = games.winner_username) AS is_winner
                   FROM sessions
                   INNER JOIN games ON sessions.game_id = games.id
                   ORDER BY sessions.score DESC
                   LIMIT 10
               )");
    QJsonArray leaderboard;

    while (query.next())
    {
        QJsonObject sessionObject;
        sessionObject["id"] = query.value("id").toInt();
        sessionObject["timestamp"] = query.value("timestamp").toString();
        sessionObject["game_id"] = query.value("game_id").toInt();
        sessionObject["player_username"] = query.value("player_username").toString();
        sessionObject["score"] = query.value("score").toInt();
        sessionObject["levels_played"] = query.value("levels_played").toInt();
        sessionObject["is_winner"] = query.value("is_winner").toBool();

        leaderboard.append(sessionObject);
    }

    query.exec(R"(
                   SELECT sessions.id, sessions.game_id, sessions.player_username, sessions.score,
                          sessions.levels_played, games.timestamp,
                          (sessions.player_username = games.winner_username) AS is_winner
                   FROM sessions
                   INNER JOIN games ON sessions.game_id = games.id
                   ORDER BY timestamp DESC
               )");
    QJsonArray sessionsArray;

    while (query.next())
    {
        QJsonObject sessionObject;
        sessionObject["id"] = query.value("id").toInt();
        sessionObject["timestamp"] = query.value("timestamp").toString();
        sessionObject["game_id"] = query.value("game_id").toInt();
        sessionObject["player_username"] = query.value("player_username").toString();
        sessionObject["score"] = query.value("score").toInt();
        sessionObject["levels_played"] = query.value("levels_played").toInt();
        sessionObject["is_winner"] = query.value("is_winner").toBool();

        sessionsArray.append(sessionObject);
    }

    QJsonObject finalData;
    finalData["type"] = "GAMEDATA";
    finalData["games"] = gamesArray;
    finalData["leaderboard"] = leaderboard;
    finalData["sessions"] = sessionsArray;

    tx(finalData);
}

void Dialog::updatePlayerPositions(QJsonArray playersArray)
{
    for (const QJsonValue &value : playersArray)
    {
        QJsonObject playerData = value.toObject();
        int clientId = playerData["clientId"].toInt();
        int x = playerData["x"].toInt();
        int y = playerData["y"].toInt();
        int angle = playerData["angle"].toInt();

        if (clientId != -1)
        {
            QJsonObject playerPosition;
            playerPosition["x"] = x;
            playerPosition["y"] = y;
            playerPosition["angle"] = angle;
            playerPositions[clientId] = playerPosition;
        }
//        qDebug() << "Updated position for client " << clientId << ": (" << x << ", " << y << ")";
    }
}

void Dialog::broadcastPlayerPositions()
{
//    qDebug() << "broadcasting player positions";
    QJsonObject positionUpdateMessage;
    positionUpdateMessage["type"] = "POSITION";

    QJsonArray playersArray;

    for (auto it = playerPositions.constBegin(); it != playerPositions.constEnd(); ++it)
    {
        QJsonObject playerData;
        playerData["clientId"] = it.key();
        playerData["x"] = it.value()["x"];
        playerData["y"] = it.value()["y"];
        playerData["angle"] = it.value()["angle"];

        playersArray.append(playerData);
    }

    positionUpdateMessage["players"] = playersArray;

    tx(positionUpdateMessage);
}

void Dialog::broadcastObstaclePositions()
{
    QJsonObject obstaclePositionMessage;
    obstaclePositionMessage["type"] = "OBSTACLE_POSITION";
    obstaclePositionMessage["obstacles"] = obstaclesArray;
    tx(obstaclePositionMessage);
}

void Dialog::broadcastActiveClients()
{
    QJsonObject activeClientsMessage;
    activeClientsMessage["type"] = "ACTIVE_CLIENTS";

    QJsonArray activeClientsArray;
    QJsonArray inGameClientsArray;

    for (auto it = clientIdMap.begin(); it != clientIdMap.end(); ++it)
    {
        const ClientData &clientData = it.value();

        QJsonObject clientObj;
        clientObj["clientId"] = clientData.clientId;
        clientObj["username"] = clientData.username;
        clientObj["color"] = clientData.color;
        clientObj["isInGame"] = clientData.isInGame;
        clientObj["isAlive"] = clientData.isAlive;
        clientObj["finishedLastLevel"] = clientData.finishedLastLevel;
        clientObj["levelsPlayed"] = clientData.levelsPlayed;
        clientObj["score"] = clientData.score;

        if (clientObj["clientId"].toInt() != -1)
        {
            activeClientsArray.append(clientObj);
            setActiveClient(clientData);

            if (clientData.isInGame)
            {
                inGameClientsArray.append(clientData.clientId);
            }
        }
    }

    activeClientsMessage["clientData"] = activeClientsArray;
    activeClientsMessage["clientIdsInGame"] = inGameClientsArray;

    tx(activeClientsMessage);

    if (inGameClientsArray.size() == 0)
    {
        activeGame = false;
    }

    if (activeClientsArray.size() > 0 && activeGame == false)
    {
        ui->startButton->setEnabled(true);
    }
    else
    {
        ui->startButton->setEnabled(false);
    }
}

void Dialog::setActiveClient(ClientData clientData)
{
    QList<QPushButton*> buttons = { ui->button1, ui->button2, ui->button3, ui->button4 };

    int index = clientData.clientId - 1;

    if (index >= 0 && index < buttons.size()) {
        qDebug() << index;
        QPushButton *button = buttons.at(index);
        button->setEnabled(true);
        button->setText(clientData.username);
        button->setStyleSheet("background-color: " + clientData.color);
    }
}

void Dialog::removeActiveClient(ClientData clientData)
{
    QList<QPushButton*> buttons = { ui->button1, ui->button2, ui->button3, ui->button4 };

    int index = clientData.clientId - 1;

    if (index >= 0 && index < buttons.size()) {
        QPushButton *button = buttons.at(index);
        button->setEnabled(false);
        button->setText("Client " + QString::number(clientData.clientId));
        button->setStyleSheet("");
    }
}

void Dialog::removeClient(QString &clientKey)
{
    qDebug() << "disconnecting " << clientKey;
    if (clientIdMap.contains(clientKey))
    {
        int clientId = clientIdMap[clientKey].clientId;
        bool isInGame = clientIdMap[clientKey].isInGame;

        if (isInGame)
        {
            clientIdsInGame.removeAll(clientId);
        }

        QStringList parts = clientKey.split(':');

        if (parts.size() == 2)
        {
            QHostAddress address = QHostAddress(parts[0]);
            quint16 port = static_cast<quint16>(parts[1].toUInt());

            int index = clientPorts.indexOf(port);
            if (index != -1)
            {
                clientAddresses.removeAt(index);
                clientPorts.removeAt(index);
                availableIds.append(clientId);
                std::sort(availableIds.begin(), availableIds.end());
                removeActiveClient(clientIdMap[clientKey]);
            }

            // create a disconnection message to broadcast
            QJsonObject disconnectionMsg;
            disconnectionMsg["type"] = "MESSAGE";
            disconnectionMsg["message"] = QString("%1 (%2:%3) has disconnected.")
                                             .arg(clientIdMap[clientKey].username)
                                             .arg(address.toString())
                                             .arg(port);

            ui->textBrowser->append(disconnectionMsg["message"].toString());
            tx(disconnectionMsg);
        }
        clientIdMap.remove(clientKey);
    }
}

void Dialog::tx(QJsonObject jsonObject)
{
    qDebug() << "tx";
    qDebug() << jsonObject;

    QJsonDocument doc(jsonObject);
    QByteArray message = doc.toJson();

    for (int i = 0; i < clientAddresses.size(); ++i)
    {
        socket->writeDatagram(message, clientAddresses[i], clientPorts[i]);
    }
}

QString Dialog::getLocalIpAddress()
{
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface &interface : interfaces)
    {
        if (interface.flags() & QNetworkInterface::IsUp)
        {
            QList<QNetworkAddressEntry> entries = interface.addressEntries();
            for (const QNetworkAddressEntry &entry : entries)
            {
                QHostAddress ip = entry.ip();
                if (ip.protocol() == QAbstractSocket::IPv4Protocol && ip != QHostAddress("127.0.0.1"))
                {
                    return ip.toString();
                }
            }
        }
    }
    return QString();
}

void Dialog::setLocalIpAddress()
{
    QString localIp = getLocalIpAddress();
    if (!localIp.isEmpty())
    {
        ui->ipEdit->setText(localIp);
    }
}


Dialog::~Dialog()
{
    delete ui;
}
