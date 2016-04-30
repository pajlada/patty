#include "ircclient.h"
#include "ircconnection.h"

#include <QSettings>
#include <IrcCommand>

IrcClient::IrcClient(QObject *parent)
    : IrcConnection(parent)
{
}

void
IrcClient::connect()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "pajlada", "patty");
    this->setHost("irc.chat.twitch.tv");
    this->setUserName(settings.value("Credentials/username", "justinfan123").toString());
    this->setNickName(settings.value("Credentials/username", "justinfan123").toString());
    this->setRealName("ASD");
    this->setPassword(settings.value("Credentials/pass", "oauth:kappa123").toString());

    this->sendCommand(IrcCommand::createJoin("#pajlada"));
    this->sendCommand(IrcCommand::createJoin("#forsenlol"));
    this->sendCommand(IrcCommand::createJoin("#mushisgosu"));
    this->sendCommand(IrcCommand::createJoin("#trumpsc"));
    this->sendCommand(IrcCommand::createJoin("#lirik"));
    this->sendCommand(IrcCommand::createJoin("#tsm_dyrus"));
    this->sendCommand(IrcCommand::createJoin("#sodapoppin"));

    this->sendCommand(IrcCommand::createCapability("REQ", "twitch.tv/commands"));
    this->sendCommand(IrcCommand::createCapability("REQ", "twitch.tv/tags"));

    this->open();
}
