#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>   // For QVBoxLayout
#include <QHBoxLayout>   // For QHBoxLayout
#include <QLineEdit>     // For QLineEdit
#include <QPushButton>   // For QPushButton
#include <QTextBrowser>  // For QTextBrowser
#include <QLabel>        // For QLabel
#include <QSpacerItem>   // For QSpacerItem (used in layout spacing)
#include "client.h"
#include "server.h"

namespace Ui {
    class Dialog;
}

class Dialog : public QDialog {
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

    Ui::Dialog *ui;

    Client* client;
    Server* server;

    QTabWidget *tabWidget;  // To hold the server and client dialogs

    // Client Tab Widgets
    QLineEdit *ipEdit;
    QLineEdit *portEdit;
    QPushButton *connectButton;
    QTextBrowser *textBrowser;
    QLineEdit *messageEdit;
    QPushButton *sendButton;

    // Server Tab Widgets (for later use if needed)
    QTextBrowser *serverBrowser;
    QLineEdit *serverIpEdit;
    QLineEdit *serverPortEdit;
    QPushButton *configureButton;

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // DIALOG_H
