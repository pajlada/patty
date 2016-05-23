#ifndef EMOTEMANAGER_H
#define EMOTEMANAGER_H

#include <QMap>
#include <QUrl>
#include <QByteArray>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QNetworkAccessManager>

class BttvEmote
{
public:
    BttvEmote(QString _hash, QString _code);

    QString hash;
    QString code;
    QRegularExpression regex;
};

class FfzEmote
{
public:
    FfzEmote(int _id, QString _code);

    int id;
    QString code;
    QRegularExpression regex;
};

class EmoteManager : public QObject
{
    Q_OBJECT

public:
    void getTwitchEmotes();

    void getBttvEmotes();
    void getFfzEmotes();

    void getBttvChannelEmotes(const QString &channel);
    void getFfzChannelEmotes(const QString &channel);

    QMap<int, QString> emote_ids;
    QList<BttvEmote> bttvEmotes;
    QList<FfzEmote> ffzEmotes;
    QMap<QString, QList<BttvEmote> > bttvChannelEmotes;
    QMap<QString, QList<FfzEmote> > ffzChannelEmotes;

    QString emote_folder;
private:
    QNetworkAccessManager network_access_manager;
};

#endif // EMOTEMANAGER_H
