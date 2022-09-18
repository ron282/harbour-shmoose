#ifndef CHATSTATENOTIFICATIONS_H
#define CHATSTATENOTIFICATIONS_H

#include <QObject>

#include <Swiften/Swiften.h>

class Persistence;
class RosterController;

class ChatStateNotifications : public QObject
{
    Q_OBJECT
public:
    ChatStateNotifications(Persistence* persistence, RosterController* rosterController, QObject *parent = 0);
    void setupWithClient(Swift::Client *client);

    static QString getMarkableString();

    void sendComposingForJid(QString jid, bool isComposing);

signals:

public slots:

private:
    void handleMessageReceived(Swift::Message::ref message);
    QString getIdFromRawXml(QString rawXml);

    Swift::Client* client_;
    Persistence* persistence_;
    RosterController* rosterController_;
};

#endif // CHATSTATENOTIFICATIONS_H
