#include "emotemanager.h"
#include "mainwindow.h"

void
EmoteManager::getTwitchEmotes()
{
    QUrl url = QUrl(QString("https://twitchemotes.com/api_cache/v2/images.json"));
    QNetworkRequest req(url);
    QNetworkReply *reply = this->network_access_manager.get(req);

    connect(reply, &QNetworkReply::finished, [=]() {
        QByteArray data = reply->readAll();

        QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
        QJsonObject root = jsonDoc.object();
        QJsonObject images = root.value("images").toObject();
        for (QString key : images.keys()) {
            QJsonObject emote = images.value(key).toObject();
            this->emote_ids.insert(key.toInt(), emote.value("code").toString());
        }
    });
}

void
EmoteManager::getBttvEmotes()
{
    QUrl url = QUrl(QString("https://api.betterttv.net/2/emotes"));
    QNetworkRequest req(url);
    QNetworkReply *reply = this->network_access_manager.get(req);

    connect(reply, &QNetworkReply::finished, [=]() {
        QByteArray data = reply->readAll();

        QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
        QJsonObject root = jsonDoc.object();
        QJsonArray emotesArray = root.value("emotes").toArray();
        for (auto it : emotesArray) {
            const QJsonObject &emote_obj = it.toObject();
            this->bttvEmotes.push_back(BttvEmote(emote_obj.value("id").toString(), emote_obj.value("code").toString()));
        }
    });
}

void
EmoteManager::getFfzEmotes()
{
    QUrl url = QUrl(QString("https://api.frankerfacez.com/v1/set/global"));
    QNetworkRequest req(url);
    QNetworkReply *reply = this->network_access_manager.get(req);

    connect(reply, &QNetworkReply::finished, [=]() {
        QByteArray data = reply->readAll();
        QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
        QJsonObject root = jsonDoc.object();
        QJsonArray default_sets = root.value("default_sets").toArray();
        for (auto it : default_sets) {
            QJsonObject set = root.value("sets").toObject().value(QString::number(it.toDouble())).toObject();
            QJsonArray emotesArray = set.value("emoticons").toArray();
            for (auto emote : emotesArray) {
                const QJsonObject &emote_obj = emote.toObject();
                this->ffzEmotes.push_back(FfzEmote(emote_obj.value("id").toInt(), emote_obj.value("name").toString()));
            }
        }
    });
}

void
EmoteManager::getBttvChannelEmotes(const QString &channel)
{
    QUrl url = QUrl(QString("https://api.betterttv.net/2/channels/%1").arg(channel.right(channel.length() - 1)));
    QNetworkRequest req(url);
    QNetworkReply *reply = this->network_access_manager.get(req);

    QList<BttvEmote> *list;

    if (this->bttvChannelEmotes.contains(channel)) {
        list = &this->bttvChannelEmotes[channel];
    } else {
        this->bttvChannelEmotes.value(channel, QList<BttvEmote>());
        list = &this->bttvChannelEmotes[channel];
    }

    connect(reply, &QNetworkReply::finished, [=]() {
        QByteArray data = reply->readAll();

        QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
        QJsonObject root = jsonDoc.object();
        QJsonArray emotesArray = root.value("emotes").toArray();
        for (auto it : emotesArray) {
            const QJsonObject &emote_obj = it.toObject();
            BttvEmote emote(emote_obj.value("id").toString(), emote_obj.value("code").toString());
            list->push_back(emote);
        }
    });
}

BttvEmote::BttvEmote(QString _hash, QString _code)
    : hash(_hash), code(_code)
{
    QString regex_str = QString("(?<![^ ])%1(?![^ ])").arg(QRegularExpression::escape(code));
    this->regex = QRegularExpression(regex_str);
}

FfzEmote::FfzEmote(int _id, QString _code)
    : id(_id), code(_code)
{
    QString regex_str = QString("(?<![^ ])%1(?![^ ])").arg(QRegularExpression::escape(code));
    this->regex = QRegularExpression(regex_str);
}
