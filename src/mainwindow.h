#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QString>
#include <QTextBrowser>
#include "emotemanager.h"
#include <QRegularExpression>

namespace Ui {
class MainWindow;
}

class IrcPrivateMessage;
class IrcJoinMessage;
class QListWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void onMessage(IrcPrivateMessage *message);
    void onJoin(IrcJoinMessage *message);
    void removeChannel(QListWidgetItem *item);
    void channelChanged();

    void switchChat(QTextBrowser* chatEdit);

    int parseLinks(QString &htmlContent);

private slots:
    void on_wSend_clicked();
    void on_cJoin_clicked();

    void on_btn_connect_clicked();

private:
    Ui::MainWindow *ui;

    EmoteManager emote_manager;

    QRegularExpression url_regex;

    QTextBrowser* currentChat;
    QMap<QString, QTextBrowser*> channelChats;
};

#endif // MAINWINDOW_H
