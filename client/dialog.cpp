#include "dialog.h"
#include "ui_dialog.h"
#include "graphicsdialog.h"
#include "defs.h"
#include <QFontDatabase>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    // set up carbon fiber texture or gradient background
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Background, QBrush(QPixmap(":/images/greenb.jpg").scaled(this->size(), Qt::KeepAspectRatioByExpanding)));
    this->setPalette(palette);

    // load Orbitron font from the resource file
    int fontId = QFontDatabase::addApplicationFont(":/images/Orbitron-VariableFont_wght.ttf");
    if (fontId != -1)
    {
       QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
       QFont orbitronFont(fontFamily, 24, QFont::Bold); // Font size 24, Bold weight

       // apply to the title label
       ui->label->setFont(orbitronFont);
       ui->label->setStyleSheet(R"(
           QLabel {
               color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 #007FFF, stop:1 #00FF80);
               text-align: center;
           }
       )");
    }
    else
    {
       qDebug() << "Failed to load Orbitron font!";
    }

    // player profile text being white, orbitron, and bold
    ui->groupBox->setStyleSheet("QGroupBox { color: white; font-family: 'Orbitron'; font-weight: bold; }");

    //making the tabwidget coooleerrrr
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
           background-color: #00FF00; /* Neon green */
           color: black;
       }

       QTabBar::tab:hover {
           background-color: #00FF80; /* lighter Neon green */
       }

       QTabWidget::tab-bar {
           alignment: center;
       }
    )");

    // styling for the text browser (retro terminal look)
    ui->textBrowser->setStyleSheet(R"(
        QTextBrowser {
            background-color: #000000;
            color: #00ff00;
            border: 2px solid #00ff00;
            font-family: "Courier New";
            font-size: 12px;
        }
    )");

    // styling for IP and Port input fields
    QString inputFieldStyle = R"(
        QLineEdit {
            border: 2px solid #00ff00;
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

    // styling for the allPlayersButton to have Orbitron font, white text initially, and neon blue text when clicked
    ui->allPlayersButton->setStyleSheet(R"(
        QRadioButton {
            color: white;
            font-family: 'Orbitron';
            font-weight: bold;
            font-size: 14px;
        }
        QRadioButton:checked {
            color: #00ffff;
        }
        QRadioButton::indicator {
            width: 16px;
            height: 16px;
        }
    )");

    // styling for the currentUserButton to have white text + other features
    ui->currentUserButton->setStyleSheet(R"(
        QRadioButton {
            color: white;
            font-family: 'Orbitron';
            font-weight: bold;
            font-size: 14px;
        }

        QRadioButton:checked {
            color: #00ffff;
        }
        QRadioButton::indicator {
            width: 16px;
            height: 16px;
        }
    )");

    // styling for the connect button
    ui->connectButton->setStyleSheet(R"(
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

    // styling for ready checkbox
    ui->checkBox->setStyleSheet(R"(
        QCheckBox {
            color: white;
        }
    )");

    setLocalIpAddress();
    ui->portEdit->setText(QString::number(DEFAULT_PORT));  // Default port

    ui->textBrowser->append("Server should send a welcome message and clear the inputs on successful connection to active game server.");

    connect(ui->usernameEdit, &QLineEdit::returnPressed, this, &Dialog::onSubmitButtonClick);
    connect(ui->messageEdit, &QLineEdit::returnPressed, this, &Dialog::onSendButtonClick);
    connect(ui->connectButton, &QPushButton::clicked, this, &Dialog::connectToServer);
    connect(ui->submitButton, &QPushButton::clicked, this, &Dialog::submitUsername);

    connect(ui->greenButton, &QPushButton::clicked, this, &Dialog::onColorButtonClick);
    connect(ui->blueButton, &QPushButton::clicked, this, &Dialog::onColorButtonClick);
    connect(ui->yellowButton, &QPushButton::clicked, this, &Dialog::onColorButtonClick);
    connect(ui->redButton, &QPushButton::clicked, this, &Dialog::onColorButtonClick);

    connect(ui->checkBox, &QCheckBox::stateChanged, this, &Dialog::setPlayerReady);

    historyModel = new QStandardItemModel();
    leaderboardModel = new QStandardItemModel();
    userHistoryModel = new QStandardItemModel();

    historyModel->setHorizontalHeaderLabels({"Timestamp", "Winner", "High Score", "Max Level"});
    leaderboardModel->setHorizontalHeaderLabels({"Timestamp", "Username", "Score", "Levels Played", "Game ID"});
    userHistoryModel->setHorizontalHeaderLabels({"Timestamp", "Score", "Levels Played", "Is Winner", "Game ID"});
    connect(ui->allPlayersButton, &QRadioButton::clicked, this, &Dialog::showAllSessions);
    connect(ui->currentUserButton, &QRadioButton::clicked, this, &Dialog::showSessionsForUser);
}

void Dialog::setGameData(QJsonObject &data)
{
    qDebug() << "setting game data";
    QJsonArray gamesArray = data["games"].toArray();
    QJsonArray leaderboardArray = data["leaderboard"].toArray();
    QJsonArray sessionsArray = data["sessions"].toArray();

    historyModel->removeRows(0, historyModel->rowCount());

    // populate game history
    for (const QJsonValue &value : gamesArray)
    {
        QJsonObject gameObject = value.toObject();
        qDebug() << gameObject;
        QString timestamp = gameObject["timestamp"].toString();
        QString winnerUsername = gameObject["winner_username"].toString();
        int highScore = gameObject["high_score"].toInt();
        int maxLevel = gameObject["max_level"].toInt();

        QList<QStandardItem *> row;
        row.append(new QStandardItem(timestamp));
        row.append(new QStandardItem(winnerUsername));
        row.append(new QStandardItem(QString::number(highScore)));
        row.append(new QStandardItem(QString::number(maxLevel)));
        historyModel->appendRow(row);
    }

    leaderboardModel->removeRows(0, leaderboardModel->rowCount());

    // populate leaderboard
    for (const QJsonValue &value : leaderboardArray)
    {
        QJsonObject sessionObject = value.toObject();
        QString playerUsername = sessionObject["player_username"].toString();
        int score = sessionObject["score"].toInt();
        int levelsPlayed = sessionObject["levels_played"].toInt();
        int gameId = sessionObject["game_id"].toInt();
        QString timestamp = sessionObject["timestamp"].toString();

        QList<QStandardItem *> row;
        row.append(new QStandardItem(timestamp));
        row.append(new QStandardItem(playerUsername));
        row.append(new QStandardItem(QString::number(score)));
        row.append(new QStandardItem(QString::number(levelsPlayed)));
        row.append(new QStandardItem(QString::number(gameId)));

        leaderboardModel->appendRow(row);
    }

    userHistoryModel->removeRows(0, userHistoryModel->rowCount());

    // populate user history
    for (const QJsonValue &value : sessionsArray)
    {
        QJsonObject sessionObject = value.toObject();
        QString sessionPlayerUsername = sessionObject["player_username"].toString();

        if (sessionPlayerUsername == playerUsername)
        {
            int score = sessionObject["score"].toInt();
            int levelsPlayed = sessionObject["levels_played"].toInt();
            bool isWinner = sessionObject["is_winner"].toBool();
            int gameId = sessionObject["game_id"].toInt();
            QString timestamp = sessionObject["timestamp"].toString();

            QList<QStandardItem *> row;
            row.append(new QStandardItem(timestamp));
            row.append(new QStandardItem(QString::number(score)));
            row.append(new QStandardItem(QString::number(levelsPlayed)));
            row.append(new QStandardItem(isWinner ? "true" : "false"));
            row.append(new QStandardItem(QString::number(gameId)));

            userHistoryModel->appendRow(row);
        }
    }

    if (ui->currentUserButton->isChecked() && !playerUsername.isEmpty())
    {
        ui->historyTableView->setModel(userHistoryModel);
    }
    else
    {
        ui->historyTableView->setModel(historyModel);
    }
    ui->historyTableView->resizeColumnsToContents();
    ui->leaderboardTableView->setModel(leaderboardModel);
    ui->leaderboardTableView->resizeColumnsToContents();
}

void Dialog::showAllSessions()
{
    ui->historyTableView->setModel(historyModel);
    ui->historyTableView->resizeColumnsToContents();
}

void Dialog::showSessionsForUser()
{
    ui->historyTableView->setModel(userHistoryModel);
    ui->historyTableView->resizeColumnsToContents();
}

void Dialog::connectToServer()
{
    bool ipHasText = !ui->ipEdit->text().isEmpty();
    bool portHasText = !ui->portEdit->text().isEmpty();

    if (!ipHasText || !portHasText) { return; }

    if (validConnection)
    {
        validConnection = false;
        leaveGame();
        disconnectFromServer();
        ui->textBrowser->clear();
//        ui->connectButton->setEnabled(true);
        ui->sendButton->setEnabled(false);
        ui->submitButton->setEnabled(false);
        ui->greenButton->setEnabled(false);
        ui->blueButton->setEnabled(false);
        ui->yellowButton->setEnabled(false);
        ui->redButton->setEnabled(false);
        ui->greenButton->setStyleSheet("");
        ui->blueButton->setStyleSheet("");
        ui->yellowButton->setStyleSheet("");
        ui->redButton->setStyleSheet("");
        ui->currentUserButton->setEnabled(false);
    }

    socket = new QUdpSocket(this);

    ip = ui->ipEdit->text();
    port = ui->portEdit->text().toUInt();

    socket->connectToHost(QHostAddress(ip), port);
    connect(socket, &QUdpSocket::readyRead, this, &Dialog::processMsg);

    if (socket->isValid())
    {
        QJsonObject connectMessage;
        connectMessage["type"] = "CONNECT";
        sendJson(connectMessage);

        connect(ui->sendButton, &QPushButton::clicked, this, &Dialog::sendMsg);
    }
}

void Dialog::closeEvent(QCloseEvent *event)
{
    qDebug() << "close event";
    if (validConnection)
    {
        leaveGame();
        disconnectFromServer();
    }
    event->accept();
}

void Dialog::disconnectFromServer()
{
    QJsonObject disconnectMessage;

    disconnectMessage["type"] = "DISCONNECT";
    sendJson(disconnectMessage);

    clientColors.clear();
    socket->disconnectFromHost();
    delete socket;
}

void Dialog::setPlayerReady()
{
    if (ui->checkBox->isChecked())
    {
        QJsonObject readyMsg;
        readyMsg["type"] = "READY";
        readyMsg["status"] = true;
        readyMsg["message"] = QString("%1 ready!").arg(playerUsername);
        sendJson(readyMsg);
    }
    else
    {
        QJsonObject readyMsg;
        readyMsg["type"] = "READY";
        readyMsg["status"] = false;
        readyMsg["message"] = QString("%1 not ready!").arg(playerUsername);
        sendJson(readyMsg);
    }
}

void Dialog::leaveGame()
{
    QJsonObject disconnectMessage;

    disconnectMessage["type"] = "LEAVE";
    sendJson(disconnectMessage);

    if (graphicsDialog)
    {
        graphicsDialog->removePlayer(activeClientId);
        graphicsDialog->close();
        delete graphicsDialog;
        graphicsDialog = nullptr;
    }
    ui->submitButton->setEnabled(true);
    ui->connectButton->setEnabled(true);
}

void Dialog::onSubmitButtonClick()
{
    ui->submitButton->click();
}

void Dialog::onSendButtonClick()
{
    ui->sendButton->click();
}

void Dialog::sendMsg()
{
    QString msg = ui->messageEdit->text();
    QJsonObject jsonMessage;
    jsonMessage["type"] = "MESSAGE";
    jsonMessage["message"] = msg;
    sendJson(jsonMessage);
    ui->messageEdit->clear();
}

void Dialog::sendJson(QJsonObject data)
{
    QJsonDocument doc(data);
    QByteArray ba = doc.toJson();

    socket->writeDatagram(ba, QHostAddress(ip), port);
}

void Dialog::processMsg()
{
    while (socket->hasPendingDatagrams())
    {
        QByteArray ba;
        ba.resize(socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        socket->readDatagram(ba.data(), ba.size(), &sender, &senderPort);

        QJsonDocument doc = QJsonDocument::fromJson(ba);
        if (!doc.isObject()) return;

        QJsonObject jsonObj = doc.object();
        QString type = jsonObj["type"].toString();
        QString message = jsonObj["message"].toString();

        qDebug() << jsonObj;

        if (type == "WELCOME")
        {
            validConnection = true;
            ui->textBrowser->setText("Connected to Server: " + ip + ":" + QString::number(port));
            clientColors.clear();
            ui->ipEdit->clear();
            ui->portEdit->clear();
            activeClientId = parseClientIdFromMsg(message);
            ui->textBrowser->append("Server: " + message);
            ui->sendButton->setEnabled(true);
//            ui->connectButton->setEnabled(false);
            ui->submitButton->setEnabled(true);
            ui->greenButton->setEnabled(true);
            ui->blueButton->setEnabled(true);
            ui->yellowButton->setEnabled(true);
            ui->redButton->setEnabled(true);
            ui->greenButton->setStyleSheet("background-color: green");
            ui->blueButton->setStyleSheet("background-color: blue");
            ui->yellowButton->setStyleSheet("background-color: yellow");
            ui->redButton->setStyleSheet("background-color: red");
            ui->currentUserButton->setEnabled(true);
        }
        else if (type == "START")
        {
            if (!graphicsDialog)
            {
                graphicsDialog = new GraphicsDialog(this);
                graphicsDialog->show();
                connect(graphicsDialog, &GraphicsDialog::requestClose, this, &Dialog::leaveGame);
            }

            if (activeClientId != -1 && graphicsDialog)
            {
                QColor color;
                if (!playerColor.isEmpty())
                {
                    color = QColor(playerColor);
                }
                else
                {
                    color = QColor("white");
                }
                graphicsDialog->addActivePlayer(activeClientId, playerUsername, color);
            }
            ui->submitButton->setEnabled(false);
            ui->connectButton->setEnabled(false);
            ui->checkBox->setChecked(false);
            ui->textBrowser->append(message);
        }
        else if (type == "DISCONNECT_ALL" || type == "KICKED")
        {
            validConnection = false;
            leaveGame();
            ui->textBrowser->clear();
            ui->connectButton->setEnabled(true);
            ui->sendButton->setEnabled(false);
            ui->submitButton->setEnabled(false);
            ui->greenButton->setEnabled(false);
            ui->blueButton->setEnabled(false);
            ui->yellowButton->setEnabled(false);
            ui->redButton->setEnabled(false);
            ui->greenButton->setStyleSheet("");
            ui->blueButton->setStyleSheet("");
            ui->yellowButton->setStyleSheet("");
            ui->redButton->setStyleSheet("");
            ui->textBrowser->append(message);
            setLocalIpAddress();
            ui->portEdit->setText(QString::number(DEFAULT_PORT));
            ui->currentUserButton->setEnabled(false);
        }
        else if (type == "REJECTION")
        {
            ui->textBrowser->append("Server: " + message);
        }
        else if (type == "ACTIVE_CLIENTS")
        {
            // process the list of active client IDs received from the server
            QJsonArray clientIdsArray = jsonObj["clientIdsInGame"].toArray();
            clientDataArray = jsonObj["clientData"].toArray();

            // set of current active client IDs received from the server
            QSet<int> newActiveClients;

            QMap<QString, QPushButton*> colorButtonMap = {
                {"green", ui->greenButton},
                {"blue", ui->blueButton},
                {"yellow", ui->yellowButton},
                {"red", ui->redButton},
            };

            for (const QJsonValue &value : clientDataArray)
            {
                clientData = value.toObject();
                int clientId = clientData["clientId"].toInt();
                QString username = clientData["username"].toString();
                QString colorString = clientData["color"].toString();

                if (clientId == activeClientId)
                {
                    playerUsername = username;
                }

                // track taken colors
                if (clientId != -1 && clientId != activeClientId && !colorString.isEmpty())
                {
                    clientColors[clientId] = colorString;
                }

                if (clientIdsArray.contains(clientId))
                {
                    if (colorString.isEmpty())
                    {
                        colorString = "white";
                    }

                    newActiveClients.insert(clientId);
                    if (graphicsDialog)
                    {
                        if (!activeClients.contains(clientId) && clientData["isAlive"].toBool())
                        {
                            graphicsDialog->addPlayer(clientId, username, QColor(colorString));
                        }
                        graphicsDialog->setPlayerState(clientData);
                    }
                }

            }

            // check for players that are no longer active
            for (int clientId : activeClients)
            {
                if (graphicsDialog)
                {
                    if (!newActiveClients.contains(clientId))
                    {
                        graphicsDialog->removePlayer(clientId);
                        graphicsDialog->setPlayerState(clientData);
                        clientColors.remove(clientId);
                    }
                    else if (!isPlayerAlive(clientId))
                    {
                        graphicsDialog->removePlayerFromScene(clientId);
                        graphicsDialog->setPlayerState(clientData);
                        clientColors.remove(clientId);
                    }
                }
            }

            activeClients = newActiveClients;

            // enable/disable color buttons based on whether the color is taken
            for (const QString &color : colorButtonMap.keys())
            {
               bool colorTaken = false;

               for (int clientId : clientColors.keys())
               {
                   if (clientColors[clientId] == color)
                   {
                       colorTaken = true;
                       break;
                   }
               }

               if (colorTaken)
               {
                   colorButtonMap[color]->setEnabled(false);
                   colorButtonMap[color]->setStyleSheet("");
               }
               else
               {
                   colorButtonMap[color]->setEnabled(true);
                   colorButtonMap[color]->setStyleSheet("background-color: " + color);
               }
            }
//            ui->textBrowser->append("Server: Active clients updated.");
        }
        else if (type == "POSITION")
        {
            QJsonArray playersArray = jsonObj["players"].toArray();
            if (graphicsDialog)
            {
                graphicsDialog->updatePlayerPositions(playersArray);
            }
        }
        else if (type == "LEVEL_OVER")
        {
            if (graphicsDialog)
            {
                graphicsDialog->handleLevelOver();
            }
            ui->textBrowser->append(message);
        }
        else if (type == "GAME_OVER")
        {
            QJsonArray resultsArray = jsonObj["results"].toArray();

            if (graphicsDialog)
            {
                graphicsDialog->handleGameOver(resultsArray);
            }
            ui->textBrowser->append(message);

            for (const QJsonValue &value : resultsArray)
            {
                QJsonObject playerResult = value.toObject();

                if (playerResult["username"].toString() == playerUsername)
                {
                    QString playerDetails = QString("Placement: %1, Score: %2, Levels Played: %3")
                                               .arg(QString::number(playerResult["placement"].toInt()))
                                               .arg(QString::number(playerResult["score"].toInt()))
                                               .arg(QString::number(playerResult["levels_played"].toInt()));
                    ui->textBrowser->append(playerDetails);
                    break;
                }
            }
            ui->textBrowser->append("Close game dialog to return to game lobby.");
        }
        else if (type == "GAMEDATA")
        {
            setGameData(jsonObj);
        }
        else if (type == "PLAYER_FINISHED")
        {
            if (graphicsDialog && activeClientId == jsonObj["clientId"].toInt())
            {
                qDebug() << "updating score";
                graphicsDialog->score = jsonObj["score"].toInt();
                graphicsDialog->display->setPlainText(QString::number(jsonObj["score"].toInt()));
            }
            ui->textBrowser->append(message);
        }
        else if (!message.isNull())
        {
            ui->textBrowser->append(message);
        }
    }
}

bool Dialog::isPlayerAlive(int clientId)
{
    for (const QJsonValue &value : clientDataArray)
    {
        QJsonObject clientData = value.toObject();
        if (clientData["clientId"].toInt() == clientId)
        {
            return clientData["isAlive"].toBool();
        }
    }
    return false;
}

int Dialog::parseClientIdFromMsg(const QString &msg)
{
    QRegExp regex("You are Client (\\d+)");
    if (regex.indexIn(msg) != -1)
    {
        return regex.cap(1).toInt();
    }
    return -1;
}

void Dialog::onColorButtonClick()
{
    QPushButton* button = static_cast<QPushButton*>(sender());

    if (!button) return;

    QString color;

    if (button == ui->greenButton)
    {
        color = "green";
    }
    else if (button == ui->blueButton)
    {
        color = "blue";
    }
    else if (button == ui->yellowButton)
    {
        color = "yellow";
    }
    else if (button == ui->redButton)
    {
        color = "red";
    }
    setPlayerColor(color);
}

void Dialog::setPlayerColor(QString &color)
{
    playerColor = color;

    QJsonObject colorMessage;

    colorMessage["type"] = "PLAYER_COLOR";
    colorMessage["clientId"] = activeClientId;
    colorMessage["color"] = playerColor;
    sendJson(colorMessage);

    ui->textBrowser->append("You selected the color: " + playerColor);
}

void Dialog::submitUsername()
{
    playerUsername = ui->usernameEdit->text();

    if (playerUsername.isEmpty())
    {
        return;
    }

    if (isUsernameTaken(playerUsername))
    {
        ui->textBrowser->append("The username '" + playerUsername + "' is already taken. Please choose another.");
        return;
    }

    QJsonObject usernameMessage;
    usernameMessage["type"] = "USERNAME";
    usernameMessage["clientId"] = activeClientId;
    usernameMessage["username"] = playerUsername;
    sendJson(usernameMessage);

    ui->textBrowser->append("You submitted the username: " + playerUsername);
}

bool Dialog::isUsernameTaken(QString username)
{
    for (const QJsonValue &value : clientDataArray)
    {
        QJsonObject clientData = value.toObject();
        if (clientData["username"].toString() == username)
        {
            return true;
        }
    }
    return false;
}

void Dialog::sendPlayerPosition(int clientId, int x, int y, int angle)
{
    QJsonObject message;
    message["type"] = "POSITION";
    QJsonArray playerPosArray;

    QJsonObject playerPosData;
    playerPosData["clientId"] = clientId;
    playerPosData["x"] = x;
    playerPosData["y"] = y;
    playerPosData["angle"] = angle;
    playerPosArray.append(playerPosData);
    message["players"] = playerPosArray;

    sendJson(message);
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
