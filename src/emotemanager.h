#ifndef EMOTEMANAGER_H
#define EMOTEMANAGER_H

#include <QImage>
#include <QMap>
#include <QNetworkAccessManager>
#include <QSignalMapper>
#include <QByteArray>
#include <QRegularExpression>

class BttvEmote
{
public:
    BttvEmote(QString _hash, QString _code);

    QString hash;
    QString code;
    QRegularExpression regex;
};

class EmoteManager : public QObject
{
    Q_OBJECT

public:
    EmoteManager();
    int get_twitch_emote(int emote_id);
    int getBttvEmote(const BttvEmote &emote);

    QMap<int, QImage> twitch_emotes;
    QMap<QString, QImage> bttvEmotesCache;

    QList<BttvEmote> bttvEmotes;

    QString emote_folder;

private:
    QNetworkAccessManager network_access_manager;

    void getBaseEmote(QString url, QMap<int, QImage> *emotes, int emote_id);
    void getBaseEmote(QString url, QMap<QString, QImage> *emotes, QString emote_id);
};

#endif // EMOTEMANAGER_H
