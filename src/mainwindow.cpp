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

    this->ui->splitter->setStretchFactor(0, 0);
    this->ui->splitter->setStretchFactor(1, 1);

    this->ui->listview_channels->resize(150,
                                        this->ui->listview_channels->height());
    this->ui->label->resize(150,
                            this->ui->label->height());
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
    QScrollBar *scrollbar = this->ui->textEdit->verticalScrollBar();
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

    QTextCursor prev_cursor = this->ui->textEdit->textCursor();
    this->ui->textEdit->moveCursor(QTextCursor::End);

    this->ui->textEdit->setTextColor(messageColor);
    this->ui->textEdit->setFontWeight(100);

    if (displayName.length() > 0) {
        this->ui->textEdit->insertPlainText(displayName);
    } else {
        this->ui->textEdit->insertPlainText(message->nick());
    }
    this->ui->textEdit->setFontWeight(0);

    if (!message->isAction()) {
        this->ui->textEdit->setTextColor(Qt::black);
        this->ui->textEdit->insertPlainText(": ");
    } else {
        this->ui->textEdit->insertPlainText(" ");
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

    this->ui->textEdit->insertHtml(html_content);

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
    write.sendCommand(IrcCommand::createMessage("#pajlada", this->ui->wInput->text()));
    this->ui->wInput->clear();
}

void MainWindow::on_btn_connect_clicked()
{
    read.connect();
    write.connect();
}
