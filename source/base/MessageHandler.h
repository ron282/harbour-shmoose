#pragma once

#include <QObject>
#include <QStringList>
#include <Swiften/Swiften.h>
#include "Settings.h"
#include "QXmppClient.h"

class DownloadManager;
class Persistence;
class ChatMarkers;
class RosterController;
class LurchAdapter;
class XMPPMessageParserClient;

class MessageHandler : public QObject
{
    Q_OBJECT
public:
    MessageHandler(Persistence* persistence, Settings * settings, RosterController* rosterController, QObject *parent = 0);

    void setupWithClient(QXmppClient *qXmppClient);

    void sendMessage(QString const &toJid, QString const &message, QString const &type, bool isGroup);
    void sendDisplayedForJid(const QString &jid);
    void downloadFile(const QString &str, const QString &msgId);

signals:
    void messageSent(QString msgId);
    void httpDownloadFinished(QString attachmentMsgId);

public slots:
    void slotAppGetsActive(bool active);
    void setAskBeforeDownloading(bool AskBeforeDownloading);

private:
#ifdef DBUS
public:
#endif
    QXmppClient *qXmppClient_;
    Persistence* persistence_;
    Settings* settings_;

    DownloadManager* downloadManager_;
    ChatMarkers* chatMarkers_;

    bool appIsActive_;
    bool askBeforeDownloading_;

    void handleMessageReceived(const QXmppMessage &message);
};
