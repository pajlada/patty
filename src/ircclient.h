#ifndef IRCCLIENT_H
#define IRCCLIENT_H

#include "pattyircmessage.h"

#include <QMap>
#include <QSet>
#include <QList>
#include <QObject>
#include <QString>
#include <QSettings>
#include <IrcCommand>
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

    QList<PattyIrcMessage*> * getMessages(const QString& channelName);

private:
    QSet<QString> channels;
    QMap<QString, QList<PattyIrcMessage*>> messages;
};

#endif // IRCCLIENT_H
