#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "emotemanager.h"

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

private slots:
    void on_wSend_clicked();
    void on_cJoin_clicked();

    void on_btn_connect_clicked();

private:
    Ui::MainWindow *ui;

    EmoteManager emote_manager;
};

#endif // MAINWINDOW_H
