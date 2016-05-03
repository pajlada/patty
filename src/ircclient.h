#ifndef IRCCLIENT_H
#define IRCCLIENT_H

#include <QSet>
#include <QString>
#include <IrcConnection>

/* Responsible for managing connections */

class IrcClient : public IrcConnection
{
    Q_OBJECT

public:
    IrcClient(QObject *parent = 0);

    void connect();

    const QSet<QString>& getChannels() const;
    bool isInChannel(const QString& channelName) const;
    bool joinChannel(const QString& channelName);

private:
    QSet<QString> channels;
};

#endif // IRCCLIENT_H
