#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = 0);
    ~LoginWindow();

private slots:
    void on_btnQuit_clicked();

    void on_btnConnect_clicked();

    void on_cbShowOAuth_stateChanged(int new_state);

private:
    Ui::LoginWindow *ui;
};

#endif // LOGINWINDOW_H
