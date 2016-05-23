#include "mainwindow.h"
#include "loginwindow.h"
#include "emotemanager.h"
#include <QApplication>
#include <QtNetwork/QSslSocket>
#include <QMessageBox>
#include <QFile>
#include <QSettings>
#include <QDebug>

int
main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile f(":qdarkstyle/style.qss");
    if (!f.exists()) {
        printf("Unable to set stylesheet, file not found\n");
    } else {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }

    if (!QSslSocket::supportsSsl()) {
        /*
        QMessageBox::information(0, "Secure Socket Client",
                                 "This system does not support OpenSSL.");
        return -1;
        */
    }

    QCoreApplication::setOrganizationName("pajlada");
    QCoreApplication::setOrganizationDomain("pajlada.se");
    QCoreApplication::setApplicationName("patty");

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "pajlada", "patty");

    int r = 0;
    MainWindow w;

    if (settings.value("Main/auto_connect", false).toBool()) {
        w.connectToIrc();
    } else {
        LoginWindow lw;
        lw.show();
        r = lw.exec();

        if (r == QDialog::Accepted) {
            w.connectToIrc();
        } else if (r == QDialog::Rejected) {
            // Quit
            return 0;
        }
    }
    w.show();

    return a.exec();
}
