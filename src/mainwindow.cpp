#include <iostream>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ircclient.h"

#include <QScrollBar>
#include <IrcCommand>
#include <IrcMessage>
#include <QTextDocumentFragment>

IrcClient client;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QObject::connect(&client,
                     &IrcConnection::privateMessageReceived,
                     this,
                     &MainWindow::onMessage);

    QObject::connect(this->ui->wInput,
                     &QLineEdit::returnPressed,
                     this->ui->wSend,
                     &QPushButton::click);
}

void
MainWindow::onMessage(IrcPrivateMessage *message)
{
    QScrollBar *scrollbar = this->ui->textEdit->verticalScrollBar();
    int cur_value = scrollbar->value();
    int max_value = scrollbar->maximum();

    QString displayName = message->tags()["display-name"].toString();
    QColor messageColor = QColor(message->tags()["color"].toString());

    QTextCursor prev_cursor = this->ui->textEdit->textCursor();
    this->ui->textEdit->moveCursor(QTextCursor::End);

    this->ui->textEdit->setTextColor(messageColor);

    if (displayName.length() > 0) {
        this->ui->textEdit->insertPlainText(displayName);
    } else {
        this->ui->textEdit->insertPlainText(message->nick());
    }

    if (!message->isAction()) {
        this->ui->textEdit->setTextColor(Qt::black);
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

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_wSend_clicked()
{
    client.sendCommand(IrcCommand::createMessage("#pajlada", this->ui->wInput->text()));
    this->ui->wInput->clear();
}

void MainWindow::on_btn_connect_clicked()
{
    client.connect();
}
