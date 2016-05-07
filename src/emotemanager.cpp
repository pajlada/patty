#include <iostream>

#include "emotemanager.h"

#include <QFile>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

EmoteManager::EmoteManager()
{
    this->emote_folder = QDir::cleanPath(
                QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                + QDir::separator() + "emotes");

    // Create the emote cache folder if it doesn't already exist
    QDir().mkpath(this->emote_folder);

    QUrl url = QUrl(QString("https://api.betterttv.net/2/emotes"));
    QNetworkRequest req(url);
    QNetworkReply *reply = this->network_access_manager.get(req);

    connect(reply,
            &QNetworkReply::finished,
            [=]() {
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
EmoteManager::getBaseEmote(QString url, QMap<int, QImage> *emotes, int emote_id)
{
    QNetworkRequest req(url);
    QNetworkReply *reply = this->network_access_manager.get(req);

    connect(reply,
            &QNetworkReply::finished,
            [=]() {
        QImage image;
        image.loadFromData(reply->readAll());
        if (image.isNull()) {
            std::cerr << "bad image" << std::endl;
        }
        image.save(QString("%1%2%3.png").arg(this->emote_folder).arg(QDir::separator()).arg(emote_id));
        emotes->insert(emote_id, image);
        });
}

void
EmoteManager::getBaseEmote(QString url, QMap<QString, QImage> *emotes, QString emote_id)
{
    QNetworkRequest req(url);
    QNetworkReply *reply = this->network_access_manager.get(req);

    connect(reply,
            &QNetworkReply::finished,
            [=]() {
        QImage image;
        QFile file(QString("%1%2%3.gif").arg(this->emote_folder).arg(QDir::separator()).arg(emote_id));
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();
        emotes->insert(emote_id, image);
        });
}

int
EmoteManager::get_twitch_emote(int emote_id)
{
    auto it = this->twitch_emotes.find(emote_id);
    if (it == this->twitch_emotes.end()) {
        // The emote ID was not found in our cache
        QString url = QString("http://static-cdn.jtvnw.net/emoticons/v1/%1/1.0").arg(emote_id);
        this->getBaseEmote(url, &this->twitch_emotes, emote_id);
        return 1;
    }

    return 0;
}

int
EmoteManager::getBttvEmote(const BttvEmote &emote)
{
    auto it = this->bttvEmotesCache.find(emote.hash);
    if (it == this->bttvEmotesCache.end()) {
        // The emote ID was not found in our cache
        QString url = QString("https://cdn.betterttv.net/emote/%1/1x").arg(emote.hash);
        this->getBaseEmote(url, &this->bttvEmotesCache, emote.hash);
        return 1;
    }

    return 0;
}

int
EmoteManager::downloadBttvEmote(const BttvEmote &emote)
{
    auto it = this->bttvEmotesCache.find(emote.hash);
    if (it == this->bttvEmotesCache.end()) {
        // The emote ID was not found in our cache
        QString url = QString("https://cdn.betterttv.net/emote/%1/1x").arg(emote.hash);
        this->getBaseEmote(url, &this->bttvEmotesCache, emote.hash);
        return 1;
    }

    return 0;
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

    connect(reply,
            &QNetworkReply::finished,
            [=]() {
        QByteArray data = reply->readAll();

        QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
        QJsonObject root = jsonDoc.object();
        QJsonArray emotesArray = root.value("emotes").toArray();
        for (auto it : emotesArray) {
            const QJsonObject &emote_obj = it.toObject();
            BttvEmote emote(emote_obj.value("id").toString(), emote_obj.value("code").toString());
            list->push_back(emote);
        }

        for (auto emote : *list) {
            this->downloadBttvEmote(emote);
        }
    });
}

BttvEmote::BttvEmote(QString _hash, QString _code)
    : hash(_hash), code(_code)
{
    QString regex_str = QString("(?<![^ ])%1(?![^ ])").arg(QRegularExpression::escape(code));
    //QString regex_str = QString("(%1)").arg(QRegularExpression::escape(code));
    this->regex = QRegularExpression(regex_str);
}
