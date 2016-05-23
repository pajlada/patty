#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "pattyircmessage.h"

#include <QMainWindow>
#include <QMap>
#include <QString>
#include <QTextBrowser>
#include <QRegularExpression>
#include <QWebFrame>

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

    void connectToIrc();

    void onMessage(IrcPrivateMessage *message);
    void addMessage(PattyIrcMessage *message);
    void onJoin(IrcJoinMessage *message);
    void removeChannel(QListWidgetItem *item);
    void channelChanged();

private slots:
    void on_wSend_clicked();
    void on_cJoin_clicked();
    void chatContentsSizeChanged(const QSize &size);
    void linkClicked(const QUrl &url);

private:
    Ui::MainWindow *ui;

    QRegularExpression message_regex;

    QString currentChannel;
    int currentMessages;

    int scrollValue;
    bool autoScroll;
};

#endif // MAINWINDOW_H
