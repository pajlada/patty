#include "pattyircmessage.h"

#define URL_REGEX "((?:https?|ftp)://\\S+)"

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
        QStringList unique_emotes = emotesString.split('/');
        QList<struct EmoteReplacement> replacements;
        for (auto unique_emote : unique_emotes) {
            int emote_id = unique_emote.section(':', 0, 0).toInt();
            int v = emote_manager.get_twitch_emote(emote_id);
            QStringList emote_occurences = unique_emote.section(':', 1, 1).split(',');
            for (auto emote_occurence : emote_occurences) {
                int begin = emote_occurence.section('-', 0, 0).toInt();
                int end = emote_occurence.section('-', 1, 1).toInt();
                QString image_tag = QString("<img src=\"file:///%1%2%3.png?v=%4\"/>").arg(emote_manager.emote_folder).arg(QDir::separator()).arg(emote_id).arg(v);
                replacements.append(EmoteReplacement {
                                        begin,
                                        end - begin + 1,
                                        image_tag
                                    });
            }
        }
        int offset = 0;
        qSort(replacements.begin(), replacements.end(), variantByIndex);
        int last_i = 0;
        for (auto replacement : replacements) {
            /* Figure out if we need to increase the offset due to
               unicode characters and HTML tag escapes. */
            for (int i=last_i+offset; i < replacement.index + offset; ++i) {
                const QChar &c = html_content[i];
                if (c.isHighSurrogate()) {
                    offset += 1;
                }
                if (c == '>' || c == '<') offset += 3; // sorry pajlada i cannot look at it otherwise
                if (c == '>') html_content = html_content.replace(i, 1, "&gt;");
                if (c == '<') html_content = html_content.replace(i, 1, "&lt;");
            }
            last_i = replacement.index + replacement.length;
            html_content = html_content.replace(replacement.index + offset,
                                 replacement.length,
                                 replacement.tag);

            offset += replacement.tag.length() - replacement.length;
        }
        for (int i = last_i+offset; i < html_content.length(); ++i) {
            const QChar &c = html_content[i];
            if (c == '>') html_content = html_content.replace(i, 1, "&gt;");
            if (c == '<') html_content = html_content.replace(i, 1, "&lt;");
        }
    } else {
        html_content = html_content.replace("<", "&lt;").replace(">", "&gt;");
    }

    parseBttvEmotes(html_content);
    parseBttvChannelEmotes(html_content, message->target());
    parseLinks(html_content);

    QString html_message = "<td class=\"message\" width=\"100%\">";
    html_message += "<span class=\"username\" style=\"color: " + msg->username_color.name() + ";\">" + msg->username;

    if (!message->isAction()) html_message += "</span>:";

    html_message += " " + html_content;

    if (message->isAction()) html_message += "</span>";
    html_message += "</td>";

    for (int i = 0; i < mention_manager.mentions.count(); ++i) { // subject to change for the future
        PattyIrcMention* mention = mention_manager.mentions.at(i);
        if (mention->regex.indexIn(message->content()) >= 0) {
            html_message = "<div class=\"mention\">" + html_message;
            html_message += "</div>";
            break;
        }
    }

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
        // 15 is the amount of extra characters added by the new html tags
        // 1 is the extra space
        offset += url.length() + 15 + 1;
    }

    return num_links;
}

void PattyIrcMessage::parseBttvEmotes(QString &htmlContent)
{
    for (auto emote_it : emote_manager.bttvEmotes) {
        const BttvEmote &emote = emote_it;

        int v = emote_manager.getBttvEmote(emote);

        QString image_tag = QString("<img src=\"file:///%1%2%3.gif?v=%4\"/>").arg(emote_manager.emote_folder).arg(QDir::separator()).arg(emote.hash).arg(v);
        htmlContent.replace(emote.regex, image_tag);
    }
}

void PattyIrcMessage::parseBttvChannelEmotes(QString &htmlContent, const QString &channel)
{
    if (emote_manager.bttvChannelEmotes.contains(channel)) {
        for (auto emote_it : emote_manager.bttvChannelEmotes[channel]) {
            const BttvEmote &emote = emote_it;

            int v = 1;

            QString image_tag = QString("<img src=\"file:///%1%2%3.gif?v=%4\"/>").arg(emote_manager.emote_folder).arg(QDir::separator()).arg(emote.hash).arg(v);
            htmlContent.replace(emote.regex, image_tag);
        }
    }
}
