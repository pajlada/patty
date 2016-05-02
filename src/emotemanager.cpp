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

EmoteManager::EmoteManager()
{
    this->emote_folder = QDir::cleanPath(
                QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                + QDir::separator() + "emotes");

    // Create the emote cache folder if it doesn't already exist
    QDir().mkpath(this->emote_folder);
}

int
EmoteManager::get_twitch_emote(int emote_id)
{
    auto it = this->twitch_emotes.find(emote_id);
    if (it == this->twitch_emotes.end()) {
        // The emote ID was not found in our cache
        QUrl url = QUrl(QString("http://static-cdn.jtvnw.net/emoticons/v1/%1/1.0").arg(emote_id));
        QNetworkRequest req(url);
        QNetworkReply *reply = this->network_access_manager.get(req);

        this->twitch_emotes_buffer[emote_id] = new QByteArray();

        connect(reply,
                &QNetworkReply::finished,
                [=]() {
            QImage image;
            image.loadFromData(reply->readAll());
            if (image.isNull()) {
                std::cerr << "bad image" << std::endl;
            }
            image.save(QString("%1%2%3.png").arg(this->emote_folder).arg(QDir::separator()).arg(emote_id));
            this->twitch_emotes.insert(emote_id, image);
        });
        return 1;
    }

    return 0;
}
