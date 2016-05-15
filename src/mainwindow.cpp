#include <iostream>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ircclient.h"
#include "emotemanager.h"

#include <QScrollBar>
#include <IrcCommand>
#include <IrcMessage>
#include <QTextDocumentFragment>
#include <QDebug>
#include <QDir>

IrcClient read;
IrcClient write;
bool connected = false;

QString default_stylesheet;

#define MESSAGE_REGEX "(<table[^>]*>.+?<\\/table>)"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    message_regex(MESSAGE_REGEX, QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption)
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
    this->ui->cInput->setEnabled(false);
    this->ui->cJoin->setEnabled(false);
    this->ui->wInput->setEnabled(false);
    this->ui->wSend->setEnabled(false);

    default_stylesheet = QString("a {"
                                 "color: #f0f;"
                                 "}"
                                 ".username {"
                                 "font-weight: bold;"
                                 "}"
                                 ".mention td {"
                                 "background-color: rgba(255, 0, 0, .4) !important;"
                                 "}");

    this->ui->baseChatWindow->document()->setDefaultStyleSheet(default_stylesheet);
    this->ui->baseChatWindow->setAnimated(true);
    currentMessages = 0;
}

void MainWindow::onMessage(IrcPrivateMessage *message) {
    auto messages = read.getMessages(message->target());
    PattyIrcMessage* pattyMsg = PattyIrcMessage::fromMessage(message);
    messages->append(pattyMsg);

    QScrollBar *scrollbar = this->ui->baseChatWindow->verticalScrollBar();
    int cur_value = scrollbar->value();
    int max_value = scrollbar->maximum();

    if (messages->count() > 150) { // Subject to change via settings, currently set as twitch default
        messages->removeAt(0);
        if (message->target() == this->currentChannel && cur_value == max_value) { // bttv behaviour
            QString html = this->ui->baseChatWindow->toHtml();
            QRegularExpressionMatchIterator it = this->message_regex.globalMatch(this->ui->baseChatWindow->toHtml());
            while (it.hasNext() && this->currentMessages > 150) {
                QRegularExpressionMatch match = it.next();
                html.replace(match.capturedStart(1), match.capturedLength(1), "");
                this->currentMessages -= 1;

            }
            this->ui->baseChatWindow->setHtml(html);

            if (cur_value == max_value) {
                // Scroll to the bottom if the user has not scrolled up
                scrollbar->setValue(scrollbar->maximum());
            } else {
                scrollbar->setValue(cur_value);
            }
        }
    }
    if (message->target() == this->currentChannel) {
        this->addMessage(pattyMsg);
    }
}

void MainWindow::addMessage(PattyIrcMessage *message) {
   QScrollBar *scrollbar = this->ui->baseChatWindow->verticalScrollBar();
   int cur_value = scrollbar->value();
   int max_value = scrollbar->maximum();
   QTextCursor prev_cursor = this->ui->baseChatWindow->textCursor();
   this->ui->baseChatWindow->moveCursor(QTextCursor::End);
   this->ui->baseChatWindow->insertHtml(message->message);
   this->currentMessages += 1;
   qDebug() << this->currentMessages;
   this->ui->baseChatWindow->setTextCursor(prev_cursor);
   if (cur_value == max_value) {
       // Scroll to the bottom if the user has not scrolled up
       scrollbar->setValue(scrollbar->maximum());
   } else {
       scrollbar->setValue(cur_value);
   }
}

void
MainWindow::onJoin(IrcJoinMessage *message)
{

    QListWidgetItem* item = new QListWidgetItem(message->channel(), this->ui->listview_channels, 0);
    this->ui->listview_channels->addItem(item);

    if (!connected) {
        connected = true;
        this->setWindowTitle("Connected");
        this->ui->cInput->setEnabled(true);
        this->ui->cJoin->setEnabled(true);
        this->currentChannel = message->channel();
    }
    PattyIrcMessage::emote_manager.getBttvChannelEmotes(message->channel());
}

void
MainWindow::removeChannel(QListWidgetItem *item)
{
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
MainWindow::channelChanged()
{
    QListWidgetItem* item = this->ui->listview_channels->currentItem();
    if (item) {
        this->currentChannel = item->text();
        this->ui->wChannel->setText(item->text());
        this->ui->wInput->setEnabled(true);
        this->ui->wSend->setEnabled(true);

        this->ui->baseChatWindow->clear();
        auto messages = read.getMessages(item->text());
        this->currentMessages = 0;
        for (int i = 0; i < messages->length(); i++) {
            this->addMessage(messages->at(i));
        }
    }
}

void
MainWindow::connectToIrc()
{
    read.connect();
    write.connect();

    // Default mention added for now, until something like loadable mentions or etc. is added
    PattyIrcMention* default_mention = new PattyIrcMention();
    default_mention->regex = QRegExp("\\b@?" + QRegExp::escape(write.nickName()) + "\\b", Qt::CaseInsensitive);
    PattyIrcMessage::mention_manager.mentions.append(default_mention);
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
    IrcCommand* join = IrcCommand::createJoin(channel.toLower());
    write.sendCommand(join);
    read.sendCommand(join);
    this->ui->cInput->clear();
}
