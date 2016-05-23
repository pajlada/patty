#include "pattyircmessage.h"
#include <QEventLoop>
#include <QWebFrame>

#define URL_REGEX "(?<![^ ])((?:https?|ftp)://\\S+)(?![^ ])"

QRegularExpression url_regex = QRegularExpression(URL_REGEX);

EmoteManager PattyIrcMessage::emote_manager;
MentionManager PattyIrcMessage::mention_manager;

PattyIrcMessage* PattyIrcMessage::fromMessage(IrcPrivateMessage* message) {
    PattyIrcMessage* msg = new PattyIrcMessage();

    QString displayName = message->tags()["display-name"].toString();
    if (displayName.length() > 0) {
        msg->username = displayName;
    } else {
        msg->username = message->nick();
    }

    QString colorString = message->tags()["color"].toString();
    if (colorString.length() == 0) {
        msg->username_color = QColor("#aa6633");
    } else {
        msg->username_color = QColor(message->tags()["color"].toString());
    }

    msg->raw_message = QString(message->toData());

    QString emotesString = message->tags()["emotes"].toString();
    QString html_content = message->content();

    if (emotesString.length() > 0) {
        parseTwitchEmotes(html_content, emotesString);
    } else {
        html_content = html_content.replace("<", "&lt;").replace(">", "&gt;");
    }

    parseBTTVEmotes(html_content);
    parseBTTVEmotes(html_content, message->target());
    parseFFZEmotes(html_content);
    parseLinks(html_content);

    QString mentionClass;

    for (int i = 0; i < mention_manager.mentions.count(); ++i) { // subject to change for the future
        PattyIrcMention* mention = mention_manager.mentions.at(i);
        if (mention->regex.indexIn(message->content()) >= 0) {
            mentionClass = QString("data-mention=\"%1\"").arg(mention->cssclass);
            break;
        }
    }

    QString html_message = QString("<div class=\"message%3\" data-emotes=\"%2\" %1>").arg(mentionClass).arg(emotesString);
    QString content = QString("<span class=\"content\" %2>%1</span>").arg(html_content);
    if (message->isAction()) {
        html_message = html_message.arg(" action");
        content = content.arg(QString("style=\"color: %1\"").arg(msg->username_color.name()));
    } else {
        html_message = html_message.arg("");
        content = content.arg("");
    }
    html_message.append(QString("<span class=\"username\" style=\"color: %1;\">%2</span>").arg(msg->username_color.name(), msg->username));

    html_message.append("<span class=\"colon\">:</span>");

    html_message.append(content);

    html_message.append("</div>");


    msg->message = html_message;

    msg->action = message->isAction();

    return msg;
}

bool PattyIrcMessage::variantByIndex(const struct EmoteReplacement &v1,
                                     const struct EmoteReplacement &v2)
{
    return v1.index < v2.index;
}

int PattyIrcMessage::parseLinks(QString &htmlContent)
{
    int num_links = 0;
    int offset = 0;

    QRegularExpressionMatchIterator it = url_regex.globalMatch(htmlContent);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();

        num_links += 1;

        QString url = match.captured(1);
        // We're adding an extra space here to make sure links don't carry over to the next line
        QString new_url = QString("<a href=\"%1\">%1</a> ").arg(url);
        htmlContent.replace(match.capturedStart(1) + offset, match.capturedLength(1), new_url);
        offset += url.length() + 15 + 1;
    }

    return num_links;
}

void PattyIrcMessage::parseTwitchEmotes(QString &message, QString &emotesString) {
    QStringList unique_emotes = emotesString.split('/');
    QList<struct EmoteReplacement> replacements;
    for (auto unique_emote : unique_emotes) {
        int emote_id = unique_emote.section(':', 0, 0).toInt();
        QStringList emote_occurences = unique_emote.section(':', 1, 1).split(',');
        for (auto emote_occurence : emote_occurences) {
            int begin = emote_occurence.section('-', 0, 0).toInt();
            int end = emote_occurence.section('-', 1, 1).toInt();
            QString image_tag = QString("<img src=\"http://static-cdn.jtvnw.net/emoticons/v1/%1/1.0\" alt=\"%2\"/>").arg(emote_id).arg(emote_manager.emote_ids[emote_id]);
            replacements.append(EmoteReplacement { begin, end - begin + 1, image_tag });
        }
    }
    int offset = 0;
    qSort(replacements.begin(), replacements.end(), variantByIndex);
    int last_i = 0;
    for (auto replacement : replacements) {
        /* Figure out if we need to increase the offset due to
           unicode characters and HTML tag escapes. */
        for (int i=last_i+offset; i < replacement.index + offset; ++i) {
            const QChar &c = message[i];
            if (c.isHighSurrogate()) {
                offset += 1;
            }
            if (c == '>' || c == '<') offset += 3; // sorry pajlada i cannot look at it otherwise
            if (c == '>') message = message.replace(i, 1, "&gt;");
            if (c == '<') message = message.replace(i, 1, "&lt;");
        }
        last_i = replacement.index + replacement.length;
        message = message.replace(replacement.index + offset, replacement.length, replacement.tag);

        offset += replacement.tag.length() - replacement.length;
    }
    for (int i = last_i+offset; i < message.length(); ++i) {
        const QChar &c = message[i];
        if (c == '>') message = message.replace(i, 1, "&gt;");
        if (c == '<') message = message.replace(i, 1, "&lt;");
    }
}

void PattyIrcMessage::parseBTTVEmotes(QString &message) {
    for (BttvEmote it : emote_manager.bttvEmotes) {
        QString image = QString("<img src=\"https://cdn.betterttv.net/emote/%1/1x\" alt=\"%2\"/>").arg(it.hash).arg(it.code);
        message.replace(it.regex, image);
    }
}

void PattyIrcMessage::parseBTTVEmotes(QString &message, const QString &channel) {
    if (emote_manager.bttvChannelEmotes.contains(channel)) {
        for (BttvEmote it : emote_manager.bttvChannelEmotes[channel]) {
            QString image = QString("<img src=\"https://cdn.betterttv.net/emote/%1/1x\" alt=\"%2\"/>").arg(it.hash, it.code);
            message.replace(it.regex, image);
        }
    }
}

void PattyIrcMessage::parseFFZEmotes(QString &message) {
    for (FfzEmote it : emote_manager.ffzEmotes) {
        QString image = QString("<img src=\"https://cdn.frankerfacez.com/emoticon/%1/1\" alt=\"%2\"/>").arg(it.id).arg(it.code);
        message.replace(it.regex, image);
    }
}

void PattyIrcMessage::parseFFZEmotes(QString &message, const QString &channel) {
    if (emote_manager.ffzChannelEmotes.contains(channel)) {
        for (FfzEmote it : emote_manager.ffzChannelEmotes[channel]) {
            QString image = QString("<img src=\"https://cdn.frankerfacez.com/emoticon/%1/1\" alt=\"%2\"/>").arg(it.id).arg(it.code);
            message.replace(it.regex, image);
        }
    }
}
