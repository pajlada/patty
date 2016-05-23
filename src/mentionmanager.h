#include <QList>
#include <QRegExp>
#include <QString>

#ifndef MENTIONMANAGER_H
#define MENTIONMANAGER_H

struct PattyIrcMention {
    QRegExp regex; // regex of mention
    QString channel; // channel where mention should popup, empty for same channel
    QList<QString> usernames; // trigger only if sent by certain username, empty for any
    QString cssclass;
};

class MentionManager {
public:
    QList<PattyIrcMention *> mentions;
};

#endif // MENTIONMANAGER_H
