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
#include <QWebElement>
#include <QDesktopServices>

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

    QObject::connect(this->ui->baseChatWindow->page()->mainFrame(),
                     &QWebFrame::contentsSizeChanged,
                     this,
                     &MainWindow::chatContentsSizeChanged);

    this->ui->splitter->setStretchFactor(0, 0);
    this->ui->splitter->setStretchFactor(1, 1);

    this->ui->listview_channels->resize(150,
                                        this->ui->listview_channels->height());
    this->ui->cInput->setEnabled(false);
    this->ui->cJoin->setEnabled(false);
    this->ui->wInput->setEnabled(false);
    this->ui->wSend->setEnabled(false);

    this->ui->baseChatWindow->settings()->setUserStyleSheetUrl(QUrl("qrc:/patty/main.css"));
    this->ui->baseChatWindow->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);//Handle link clicks by yourself
    this->ui->baseChatWindow->setContextMenuPolicy(Qt::NoContextMenu);

    QObject::connect(this->ui->baseChatWindow,
                     &QWebView::linkClicked,
                     this,
                     &MainWindow::linkClicked);

    PattyIrcMessage::emote_manager.getTwitchEmotes();
    PattyIrcMessage::emote_manager.getBttvEmotes();
    PattyIrcMessage::emote_manager.getFfzEmotes();
}

void MainWindow::onMessage(IrcPrivateMessage *message) {
    auto messages = read.getMessages(message->target());
    int messageCount = 150; // Subject to change via settings, currently set as twitch default
    PattyIrcMessage* pattyMsg = PattyIrcMessage::fromMessage(message);
    messages->append(pattyMsg);

    if (message->target() == this->currentChannel) {
        QString html;
        QWebFrame* frame = this->ui->baseChatWindow->page()->mainFrame();
        this->scrollValue = frame->scrollBarValue(Qt::Vertical);
        this->autoScroll = (this->scrollValue >= frame->scrollBarMaximum(Qt::Vertical));
        if (this->autoScroll) {
            while (messages->count() > messageCount) {
                delete messages->takeFirst();
                QWebElement message = frame->findFirstElement("div.message");
                message.removeFromDocument();
            }
        }
        addMessage(pattyMsg);
    } else {
        if (messages->count() > messageCount) {
            delete messages->takeFirst();
        }
    }
}

void
MainWindow::addMessage(PattyIrcMessage *message) {
    QWebElement pageBody = this->ui->baseChatWindow->page()->mainFrame()->documentElement().findFirst("body");
    pageBody.appendInside(message->message);
}

void
MainWindow::chatContentsSizeChanged(const QSize &size) {
    // webkit is dumb ok
    QWebFrame* frame = this->ui->baseChatWindow->page()->mainFrame();
    if (this->autoScroll) {
        frame->setScrollBarValue(Qt::Vertical, size.height());
    } else {
        frame->setScrollBarValue(Qt::Vertical, this->scrollValue);
    }
}

void
MainWindow::linkClicked(const QUrl &url) {
    QDesktopServices::openUrl(url);
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

        auto messages = read.getMessages(item->text());

        QString html;
        QWebFrame* frame = this->ui->baseChatWindow->page()->mainFrame();
        frame->setHtml("");
        for (int i = 0; i < messages->count(); ++i) {
            auto message = messages->at(i);
            addMessage(message);
        }
        this->scrollValue = 0;
        this->autoScroll = true;
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
    default_mention->cssclass = "default";
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
