#include "loginwindow.h"
#include "ui_loginwindow.h"

#include <QSettings>
#include <QDebug>
#include <QCheckBox>

LoginWindow::LoginWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "pajlada", "patty");
    this->ui->cbAutoConnect->setChecked(settings.value("Main/auto_connect", false).toBool());
    this->ui->leUsername->setText(settings.value("Credentials/username").toString());
    this->ui->leOauth->setText(settings.value("Credentials/password").toString());

    QObject::connect(this->ui->btnConnect, SIGNAL(clicked()), this, SLOT(accept()));
    QObject::connect(this->ui->btnQuit, SIGNAL(clicked()), this, SLOT(reject()));
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::on_btnQuit_clicked()
{
    this->hide();
}

void LoginWindow::on_btnConnect_clicked()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "pajlada", "patty");
    settings.setValue("Main/auto_connect", this->ui->cbAutoConnect->isChecked());
    settings.setValue("Credentials/username", this->ui->leUsername->text());
    settings.setValue("Credentials/password", this->ui->leOauth->text());

    this->hide();

}

void LoginWindow::on_cbShowOAuth_stateChanged(int new_state)
{
    qDebug() << "state changeD: " << new_state;
    if (new_state == Qt::CheckState::Checked) {
        this->ui->leOauth->setEchoMode(QLineEdit::Normal);
    } else {
        this->ui->leOauth->setEchoMode(QLineEdit::Password);
    }
}
