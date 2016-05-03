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

    currentChat = this->ui->textEdit;
}

struct EmoteReplacement
{
    int index;
    int length;
    QString tag;
};

bool
variantByIndex(const struct EmoteReplacement &v1,
               const struct EmoteReplacement &v2)
{
    return v1.index < v2.index;
}

void
MainWindow::onMessage(IrcPrivateMessage *message)
{
    QMap<QString, QTextEdit*>::iterator itr = this->channelChats.find(message->target());
    if (itr == this->channelChats.end())
    {
        // @todo: Private messages are going to have the nickname in target() probably.
        return;
    }

    QTextEdit* channelChat = *itr;

    QScrollBar *scrollbar = channelChat->verticalScrollBar();
    int cur_value = scrollbar->value();
    int max_value = scrollbar->maximum();

    QString displayName = message->tags()["display-name"].toString();
    QString colorString = message->tags()["color"].toString();
    QString emotesString = message->tags()["emotes"].toString();
    QColor messageColor;
    if (colorString.length() == 0) {
        messageColor = QColor("#aa6633");
    } else {
        messageColor = QColor(message->tags()["color"].toString());
    }

    QTextCursor prev_cursor = channelChat->textCursor();
    channelChat->moveCursor(QTextCursor::End);

    channelChat->setTextColor(messageColor);
    channelChat->setFontWeight(99);

    if (displayName.length() > 0) {
        channelChat->insertPlainText(displayName);
    } else {
        channelChat->insertPlainText(message->nick());
    }
    channelChat->setFontWeight(0);

    if (!message->isAction()) {
        channelChat->setTextColor(Qt::gray);
        channelChat->insertPlainText(": ");
    } else {
        channelChat->insertPlainText(" ");
    }

    const QString &content = message->content();
    QString html_content(content);

    if (emotesString.length() > 0) {
        QStringList unique_emotes = emotesString.split('/');
        QList<struct EmoteReplacement> replacements;
        for (auto unique_emote : unique_emotes) {
            int emote_id = unique_emote.section(':', 0, 0).toInt();
            int v = emote_manager.get_twitch_emote(emote_id);
            QStringList emote_occurences = unique_emote.section(':', 1, 1).split(',');
            for (auto emote_occurence : emote_occurences) {
                int begin = emote_occurence.section('-', 0, 0).toInt();
                int end = emote_occurence.section('-', 1, 1).toInt();
                QString image_tag = QString("<img src=\"file:///%1%2%3.png?v=%4\"/>").arg(emote_manager.emote_folder).arg(QDir::separator()).arg(emote_id).arg(v);
                replacements.append(EmoteReplacement {
                                        begin,
                                        end - begin + 1,
                                        image_tag
                                    });
            }
        }

        int offset = 0;
        qSort(replacements.begin(), replacements.end(), variantByIndex);
        int last_i = 0;
        for (auto replacement : replacements) {
            /* Figure out if we need to increase the offset due to
               unicode characters. */
            for (int i=last_i+offset; i < replacement.index + offset; ++i) {
                const QChar &c = html_content[i];
                if (c.isHighSurrogate()) {
                    // qDebug() << "offset += 1 due to high surrogate";
                    offset += 1;
                }
            }
            last_i = replacement.index;
            html_content = html_content.replace(replacement.index + offset,
                                 replacement.length,
                                 replacement.tag);

            offset += replacement.tag.length() - replacement.length;
        }
    }

    channelChat->insertHtml(html_content);

    channelChat->insertPlainText("\n");

    channelChat->setTextCursor(prev_cursor);

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

    QTextEdit* chatTextEdit = new QTextEdit();
    chatTextEdit->setReadOnly(true);
    chatTextEdit->setGeometry(this->ui->textEdit->geometry());
    if (this->channelChats.size() == 0)
        switchChat(chatTextEdit);

    this->channelChats.insert(message->channel(), chatTextEdit);
}

void
MainWindow::removeChannel(QListWidgetItem *item) {
    IrcCommand* part = IrcCommand::createPart(item->text());
    write.sendCommand(part);
    read.sendCommand(part);
    if (this->ui->listview_channels->count() == 0) {
        this->ui->wChannel->setText("");
        this->ui->wInput->setEnabled(false);
        this->ui->wSend->setEnabled(false);
    }

    this->channelChats.remove(item->text());
    delete item;
}

void
MainWindow::channelChanged() {
    QListWidgetItem* item = this->ui->listview_channels->currentItem();
    if (item) {
        this->ui->wChannel->setText(item->text());
        this->ui->wInput->setEnabled(true);
        this->ui->wSend->setEnabled(true);

        QMap<QString, QTextEdit*>::iterator itr = this->channelChats.find(item->text());
        if (itr != this->channelChats.end())
            switchChat(*itr);
    }
}

void
MainWindow::switchChat(QTextEdit* chatEdit) {
    if (this->currentChat)
    {
        this->currentChat->hide();
        chatEdit->setGeometry(this->currentChat->geometry());
        chatEdit->setSizePolicy(this->currentChat->sizePolicy());
        this->ui->verticalLayout->removeWidget(this->currentChat);
        this->ui->verticalLayout->insertWidget(1, chatEdit);
        this->currentChat = chatEdit;
        this->currentChat->show();
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
