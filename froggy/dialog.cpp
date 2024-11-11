#include "dialog.h"
#include "ui_dialog.h"
#include "server.h"
#include "client.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog) {

    // Create and setup the QLabel for the bold text
    QLabel *titleLabel = new QLabel("Fast and Froggy Game Host", this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);  // Set text to bold
    titleFont.setPointSize(14);  // Set font size to 14
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);  // Center the text

    // Create the QTabWidget
    tabWidget = new QTabWidget(this);

    // Create client and server tabs
    QWidget *clientTab = new QWidget(tabWidget);
    QWidget *serverTab = new QWidget(tabWidget);

    // Layout for client tab
    QVBoxLayout *clientLayout = new QVBoxLayout(clientTab);

    QVBoxLayout *clientInnerLayout = new QVBoxLayout();
    QHBoxLayout *clientTopLayout = new QHBoxLayout();

    // Client IP input
    ipEdit = new QLineEdit(clientTab);
    ipEdit->setPlaceholderText("Enter IP Address Here");
    clientTopLayout->addWidget(ipEdit);

    // Client Port input
    portEdit = new QLineEdit(clientTab);
    portEdit->setPlaceholderText("Enter Port # Here");
    clientTopLayout->addWidget(portEdit);

    // Connect button
    connectButton = new QPushButton("Connect", clientTab);
    clientTopLayout->addWidget(connectButton);

    // Add top layout to inner layout
    clientInnerLayout->addLayout(clientTopLayout);

    // Message display area
    textBrowser = new QTextBrowser(clientTab);
    clientInnerLayout->addWidget(textBrowser);

    // Message input and Send button
    QHBoxLayout *messageLayout = new QHBoxLayout();
    messageEdit = new QLineEdit(clientTab);
    messageEdit->setPlaceholderText("Enter Message Here");
    messageLayout->addWidget(messageEdit);

    sendButton = new QPushButton("Send", clientTab);
    sendButton->setEnabled(false);  // Disabled initially
    messageLayout->addWidget(sendButton);

    clientInnerLayout->addLayout(messageLayout);

    // Add the inner layout to the main client layout
    clientLayout->addLayout(clientInnerLayout);

    // Set up the client tab
    tabWidget->addTab(clientTab, "Client");

    // Layout for server tab
    QVBoxLayout *serverLayout = new QVBoxLayout(serverTab);

    // Server message display area
    serverBrowser = new QTextBrowser(serverTab);
    serverLayout->addWidget(serverBrowser);

    // IP input for server
    QHBoxLayout *serverConfigLayout = new QHBoxLayout();
    serverIpEdit = new QLineEdit(serverTab);
    serverIpEdit->setPlaceholderText("Enter IP Address Here");
    serverConfigLayout->addWidget(serverIpEdit);

    // Port input for server
    serverPortEdit = new QLineEdit(serverTab);
    serverPortEdit->setPlaceholderText("Enter Port # Here");
    serverConfigLayout->addWidget(serverPortEdit);

    // Configure button for the server
    configureButton = new QPushButton("Configure", serverTab);
    serverConfigLayout->addWidget(configureButton);

    serverLayout->addLayout(serverConfigLayout);

    // Add the server tab
    tabWidget->addTab(serverTab, "Server");

    // Set up the main layout for the dialog
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(tabWidget);

    // Set the layout for the dialog
    setLayout(mainLayout);

    ui->setupUi(this);
    this->resize(500, 500);

    client = new Client(this);
    server = new Server(this);

    connect(connectButton, &QPushButton::clicked, client, &Client::connectToServer);
    connect(configureButton, &QPushButton::clicked, server, &Server::configureServer);
}

void Dialog::closeEvent(QCloseEvent *event)
{
    event->accept();
    if (client)
    {
        client->closeEvent(event);
    }
}

Dialog::~Dialog()
{
    delete ui;
}
