#include <iostream>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ircclient.h"

#include <QScrollBar>
#include <IrcCommand>
#include <IrcMessage>
#include <QTextDocumentFragment>

IrcClient read;
IrcClient write;
bool connected = false;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QObject::connect(&read,
                     &IrcConnection::privateMessageReceived,
                     this,
                     &MainWindow::onMessage);

    QObject::connect(this->ui->wInput,
                     &QLineEdit::returnPressed,
                     this->ui->wSend,
                     &QPushButton::click);

    QObject::connect(&read,
                     &IrcConnection::joinMessageReceived,
                     this,
                     &MainWindow::onJoin);

    QObject::connect(this->ui->listview_channels,
                     &QListWidget::itemDoubleClicked,
                     this,
                     &MainWindow::removeChannel);

    QObject::connect(this->ui->listview_channels,
                     &QListWidget::itemSelectionChanged,
                     this,
                     &MainWindow::channelChanged);

    QObject::connect(this->ui->cInput,
                     &QLineEdit::returnPressed,
                     this->ui->cJoin,
                     &QPushButton::click);

    this->ui->splitter->setStretchFactor(0, 0);
    this->ui->splitter->setStretchFactor(1, 1);

    this->ui->listview_channels->resize(150,
                                        this->ui->listview_channels->height());
    this->ui->label->resize(150,
                            this->ui->label->height());
    this->ui->cInput->setEnabled(false);
    this->ui->cJoin->setEnabled(false);
    this->ui->wInput->setEnabled(false);
    this->ui->wSend->setEnabled(false);
}

void
MainWindow::onMessage(IrcPrivateMessage *message)
{
    QScrollBar *scrollbar = this->ui->textEdit->verticalScrollBar();
    int cur_value = scrollbar->value();
    int max_value = scrollbar->maximum();

    QString displayName = message->tags()["display-name"].toString();
    QString colorString = message->tags()["color"].toString();
    QColor messageColor;
    if (colorString.length() == 0) {
        messageColor = QColor("#aa6633");
    } else {
        messageColor = QColor(message->tags()["color"].toString());
    }

    QTextCursor prev_cursor = this->ui->textEdit->textCursor();
    this->ui->textEdit->moveCursor(QTextCursor::End);

    this->ui->textEdit->setTextColor(messageColor);
    this->ui->textEdit->setFontWeight(99);

    if (displayName.length() > 0) {
        this->ui->textEdit->insertPlainText(displayName);
    } else {
        this->ui->textEdit->insertPlainText(message->nick());
    }
    this->ui->textEdit->setFontWeight(0);

    if (!message->isAction()) {
        this->ui->textEdit->setTextColor(Qt::gray);
        this->ui->textEdit->insertPlainText(": ");
    } else {
        this->ui->textEdit->insertPlainText(" ");
    }

    this->ui->textEdit->insertPlainText(message->content());

    // insert image. not sure if this is how we'll do it
    // this->ui->textEdit->insertHtml("<img src=\"file:///D:\\data\\pictures\\Kappa.png\">");

    this->ui->textEdit->insertPlainText("\n");

    this->ui->textEdit->setTextCursor(prev_cursor);

    if (cur_value == max_value) {
        // Scroll to the bottom if the user has not scrolled up
        scrollbar->setValue(scrollbar->maximum());
    } else {
        scrollbar->setValue(cur_value);
    }
}

void
MainWindow::onJoin(IrcJoinMessage *message) {
    if (!connected) {
        this->setWindowTitle("Connected");
        this->ui->cInput->setEnabled(true);
        this->ui->cJoin->setEnabled(true);
        connected = false;
    }
    QListWidgetItem* item = new QListWidgetItem(message->channel(), this->ui->listview_channels, 0);
    this->ui->listview_channels->addItem(item);
}

void
MainWindow::removeChannel(QListWidgetItem *item) {
    IrcCommand* part = IrcCommand::createPart(item->text());
    write.sendCommand(part);
    read.sendCommand(part);
    delete item;
    if (this->ui->listview_channels->count() == 0) {
        this->ui->wChannel->setText("");
        this->ui->wInput->setEnabled(false);
        this->ui->wSend->setEnabled(false);
    }
}

void
MainWindow::channelChanged() {
    QListWidgetItem* item = this->ui->listview_channels->currentItem();
    if (item) {
        this->ui->wChannel->setText(item->text());
        this->ui->wInput->setEnabled(true);
        this->ui->wSend->setEnabled(true);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_wSend_clicked()
{
    write.sendCommand(IrcCommand::createMessage(this->ui->wChannel->text(), this->ui->wInput->text()));
    this->ui->wInput->clear();
}

void MainWindow::on_cJoin_clicked()
{
    QString channel = this->ui->cInput->text();
    if (!channel.startsWith("#")) {
        channel = "#" + channel;
    }
    QList<QListWidgetItem *> list = this->ui->listview_channels->findItems(channel, Qt::MatchCaseSensitive);
    if (list.length() > 0) {
        return;
    }
    IrcCommand* join = IrcCommand::createJoin(channel);
    write.sendCommand(join);
    read.sendCommand(join);
    this->ui->cInput->clear();
}

void MainWindow::on_btn_connect_clicked()
{
    read.connect();
    write.connect();
}
