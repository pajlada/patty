#include "mainwindow.h"
#include <QApplication>
#include <QtNetwork/QSslSocket>
#include <QMessageBox>
#include <QFile>

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

    MainWindow w;
    w.show();

    return a.exec();
}
