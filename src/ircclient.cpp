#include "ircclient.h"
#include "ircconnection.h"

#include <QSettings>
#include <IrcCommand>

IrcClient::IrcClient(QObject *parent)
    : IrcConnection(parent),
    channels()
{
}

void
IrcClient::connect()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "pajlada", "patty");
    this->setHost("irc.chat.twitch.tv");
    this->setUserName(settings.value("Credentials/username", "justinfan123").toString());
    this->setNickName(settings.value("Credentials/username", "justinfan123").toString());
    this->setRealName("Patty User");
    this->setPassword(settings.value("Credentials/password", "oauth:kappa123").toString());

    this->joinChannel("#pajlada");
    this->joinChannel("#forsenlol");
    this->joinChannel("#mushisgosu");
    this->joinChannel("#trumpsc");
    this->joinChannel("#lirik");
    this->joinChannel("#tsm_dyrus");
    this->joinChannel("#sodapoppin");

    this->sendCommand(IrcCommand::createCapability("REQ", "twitch.tv/commands"));
    this->sendCommand(IrcCommand::createCapability("REQ", "twitch.tv/tags"));

    this->open();
}

const QSet<QString>&
IrcClient::getChannels() const
{
    return this->channels;
}

bool
IrcClient::isInChannel(const QString& channelName) const
{
    return this->channels.contains(channelName);
}

bool
IrcClient::joinChannel(const QString& channelName)
{
    if (this->channels.contains(channelName))
        return false;

    if (this->sendCommand(IrcCommand::createJoin(channelName)))
    {
       this->channels.insert(channelName);
       return true;
    }

    return false;
}
