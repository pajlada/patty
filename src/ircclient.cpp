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

    this->setHost(settings.value("Server/host", "irc.chat.twitch.tv").toString());
    this->setPort(settings.value("Server/port", 6667).toInt());

    this->setPassword(settings.value("Credentials/password", "oauth:kappa123").toString());
    this->setUserName(settings.value("Credentials/username", "justinfan123").toString());
    this->setNickName(settings.value("Credentials/username", "justinfan123").toString());
    this->setRealName("Patty User");

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
