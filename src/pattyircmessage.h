#include "emotemanager.h"
#include "mentionmanager.h"

#include <QString>
#include <QColor>
#include <IrcCore>
#include <QDir>
#include <QStringList>
#include <QList>
#include <QWebElement>

#ifndef PATTYIRCMESSAGE_H
#define PATTYIRCMESSAGE_H

class IrcPrivateMessage;

struct PattyIrcMessage {
public:
    // Username with proper capitalization
    QString username;

    // Username color
    QColor username_color;

    // Raw message just as it was received from IRC
    QString raw_message;

    // Full parsed HTML Message (with emotes etc)
    QString message;

    // Whether it's a /me or not
    bool action;

    // Flags parsed from IRCv3 tags
    bool moderator;
    bool staff;
    bool broadcaster;
    bool bot;

    static PattyIrcMessage* fromMessage(IrcPrivateMessage* message);
    static bool variantByIndex(const struct EmoteReplacement &v1, const struct EmoteReplacement &v2);
    static int parseLinks(QString &htmlContent);
    static void parseTwitchEmotes(QString &message, QString &emotesString);
    static void parseBTTVEmotes(QString &message);
    static void parseBTTVEmotes(QString &message, const QString &channel);
    static void parseFFZEmotes(QString &message);
    static void parseFFZEmotes(QString &message, const QString &channel);

    static EmoteManager emote_manager;
    static MentionManager mention_manager;
};

struct EmoteReplacement
{
    int index;
    int length;
    QString tag;
};

#endif // PATTYIRCMESSAGE_H
