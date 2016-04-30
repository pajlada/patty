#ifndef IRCCLIENT_H
#define IRCCLIENT_H

#include <IrcConnection>

/* Responsible for managing connections */

class IrcClient : public IrcConnection
{
    Q_OBJECT

public:
    IrcClient(QObject *parent = 0);

    void connect();
};

#endif // IRCCLIENT_H
