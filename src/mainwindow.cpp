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

#define URL_REGEX "((?:https?|ftp)://\\S+)"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    url_regex(URL_REGEX, QRegularExpression::CaseInsensitiveOption)
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

    currentChat = this->ui->baseChatWindow;
    currentChat->document()->setDefaultStyleSheet(default_stylesheet);
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
    auto itr = this->channelChats.find(message->target());
    if (itr == this->channelChats.end())
    {
        // @todo: Private messages are going to have the nickname in target() probably.
        return;
    }

    QTextBrowser* channelChat = *itr;

    QScrollBar *scrollbar = channelChat->verticalScrollBar();
    int cur_value = scrollbar->value();
    int max_value = scrollbar->maximum();

    QString displayName = message->tags()["display-name"].toString();
    if (displayName.length() == 0) {
        displayName = message->nick();
    }
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
                // ANTI-PAJLADA-TRIGGER ACTIVATE BEEP BOOP
                if (c == '>' || c == '<') {
                    offset += 3;
                }
                if (c == '>') {
                    html_content = html_content.replace(i, 1, "&gt;");
                }
                if (c == '<') {
                    html_content = html_content.replace(i, 1, "&lt;");
                }
            }
            last_i = replacement.index + replacement.length;
            html_content = html_content.replace(replacement.index + offset,
                                 replacement.length,
                                 replacement.tag);

            offset += replacement.tag.length() - replacement.length;
        }
        for (int i = last_i+offset; i < html_content.length(); ++i) {
            const QChar &c = html_content[i];
            // ANTI-PAJLADA-TRIGGER ACTIVATE BEEP BOOP
            if (c == '>') {
                html_content = html_content.replace(i, 1, "&gt;");
            }
            if (c == '<') {
                html_content = html_content.replace(i, 1, "&lt;");
            }
        }
    } else {
        html_content = html_content.replace("<", "&lt;").replace(">", "&gt;");
    }
    QString html_message = "<td class=\"message\" width=\"100%\">";
    html_message += "<span class=\"username\" style=\"color: " + messageColor.name() + ";\">" + displayName;

    if (!message->isAction()) html_message += "</span>:";

    this->parseLinks(html_content);
    html_message += " " + html_content;
    for (int i = 0; i < this->mentions.count(); ++i) {
        auto mention = this->mentions.at(i);
        if (mention.indexIn(content) >= 0) {
            html_message = "<div class=\"mention\">" + html_message;
            html_message += "</div>";
        }
    }
    if (message->isAction()) html_message += "</span>";
    html_message += "</td>";

    channelChat->insertHtml(html_message);

    channelChat->setTextCursor(prev_cursor);

    if (cur_value == max_value) {
        // Scroll to the bottom if the user has not scrolled up
        scrollbar->setValue(scrollbar->maximum());
    } else {
        scrollbar->setValue(cur_value);
    }
}

int
MainWindow::parseLinks(QString &htmlContent)
{
    int num_links = 0;
    int offset = 0;

    QRegularExpressionMatchIterator it = this->url_regex.globalMatch(htmlContent);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();

        num_links += 1;

        QString url = match.captured(1);
        // We're adding an extra space here to make sure links don't carry over to the next line
        QString new_url = QString("<a href=\"%1\">%1</a> ").arg(url);
        htmlContent.replace(match.capturedStart(1) + offset, match.capturedLength(1), new_url);
        // 15 is the amount of extra characters added by the new html tags
        // 1 is the extra space
        offset += url.length() + 15 + 1;
    }

    return num_links;
}

void
MainWindow::onJoin(IrcJoinMessage *message)
{
    if (!connected) {
        this->setWindowTitle("Connected");
        this->ui->cInput->setEnabled(true);
        this->ui->cJoin->setEnabled(true);
        connected = false;
    }
    QListWidgetItem* item = new QListWidgetItem(message->channel(), this->ui->listview_channels, 0);
    this->ui->listview_channels->addItem(item);

    QTextBrowser* chatTextEdit = new QTextBrowser(this);
    chatTextEdit->setReadOnly(true);
    chatTextEdit->setGeometry(this->ui->baseChatWindow->geometry());
    chatTextEdit->setOpenExternalLinks(true);
    chatTextEdit->document()->setDefaultStyleSheet(default_stylesheet);
    if (this->channelChats.size() == 0) {
        switchChat(chatTextEdit);
    }

    this->channelChats.insert(message->channel(), chatTextEdit);
}

void
MainWindow::removeChannel(QListWidgetItem *item)
{
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
MainWindow::channelChanged()
{
    QListWidgetItem* item = this->ui->listview_channels->currentItem();
    if (item) {
        this->ui->wChannel->setText(item->text());
        this->ui->wInput->setEnabled(true);
        this->ui->wSend->setEnabled(true);

        auto itr = this->channelChats.find(item->text());
        if (itr != this->channelChats.end()) {
            switchChat(*itr);
        }
    }
}

void
MainWindow::connectToIrc()
{
    read.connect();
    write.connect();
    // Default mention added for now, until something like loadable mentions or etc. is added
    this->mentions.append(QRegExp("\\b@?" + QRegExp::escape(write.nickName()) + "\\b", Qt::CaseInsensitive));
}

void
MainWindow::switchChat(QTextBrowser *chatEdit)
{
    if (this->currentChat) {
        this->currentChat->hide();
        /*
        chatEdit->setGeometry(this->currentChat->geometry());
        chatEdit->setSizePolicy(this->currentChat->sizePolicy());
        */
        this->ui->verticalLayout->removeWidget(this->currentChat);
    }

    this->ui->verticalLayout->insertWidget(0, chatEdit);
    this->currentChat = chatEdit;
    this->currentChat->show();
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
