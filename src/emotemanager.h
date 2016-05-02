#ifndef EMOTEMANAGER_H
#define EMOTEMANAGER_H

#include <QImage>
#include <QMap>
#include <QNetworkAccessManager>
#include <QSignalMapper>
#include <QByteArray>

class EmoteManager : public QObject
{
    Q_OBJECT

public:
    EmoteManager();
    int EmoteManager::get_twitch_emote(int emote_id);

    QMap<int, QImage> twitch_emotes;

    QString emote_folder;

private:
    QMap<int, QByteArray*> twitch_emotes_buffer;
    QNetworkAccessManager network_access_manager;
};

#endif // EMOTEMANAGER_H
