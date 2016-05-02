#include "mainwindow.h"
#include <QApplication>
#include <QtNetwork/QSslSocket>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setStyleSheet("QSplitter::handle { background-color: gray }");

    //qputenv("IRC_DEBUG", "1");

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
