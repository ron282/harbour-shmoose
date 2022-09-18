#include "MessageHandler.h"
#include "Persistence.h"
#include "ImageProcessing.h"
#include "DownloadManager.h"
#include "ChatMarkers.h"
#include "XmlProcessor.h"
#include "RosterController.h"

#include "QXmppMessage.h"
#include "QXmppUtils.h"


#include <QUrl>
#include <QDebug>
#include <QMimeDatabase>

MessageHandler::MessageHandler(Persistence *persistence, Settings * settings, RosterController* rosterController, QObject *parent) : QObject(parent),
    qXmppClient_(nullptr), persistence_(persistence), settings_(settings),
    downloadManager_(new DownloadManager(this)),
    chatMarkers_(new ChatMarkers(persistence_, rosterController, this)),
    appIsActive_(true)
{
    connect(settings_, SIGNAL(askBeforeDownloadingChanged(bool)), this, SLOT(setAskBeforeDownloading(bool)));
    connect(downloadManager_, SIGNAL(httpDownloadFinished(QString)), this, SIGNAL(httpDownloadFinished(QString)));
}

void MessageHandler::setupWithClient(QXmppClient* qXmppClient)
{
    if (qXmppClient != nullptr)
    {
        qXmppClient_ = qXmppClient;

        chatMarkers_->setupWithClient(qXmppClient_);
        connect(qXmppClient_, &QXmppClient::messageReceived, this, &MessageHandler::handleMessageReceived);
        setAskBeforeDownloading(settings_->getAskBeforeDownloading());
    }
}

void MessageHandler::handleMessageReceived(const QXmppMessage &message)
{
    unsigned int security = 0;
    if(message.encryptionMethod() == QXmppMessage::OMEMO)
    {
        security = 1;
    }

    // XEP 280
    bool sentCarbon = false;

    // If this is a carbon message, we need to retrieve the actual content
    if (message.isCarbonForwarded() == true)
    {
        if(qXmppClient_->configuration().jidBare().compare(QXmppUtils::jidToBareJid(message.from()), Qt::CaseInsensitive) == 0)
        {
            sentCarbon = true;
        }
    }

    if (! message.body().isEmpty())
    {
        QUrl oobUrl(message.outOfBandUrl());
        bool isLink = false;

        QString type = "txt";
        QString messageId = message.id();

        if (! message.outOfBandUrl().isEmpty())  // it's an url
        {
            isLink = true;
            //isLink = security == 1 ? theBody.startsWith("aesgcm://") : isLink;

            if(isLink)
            {
                type = QMimeDatabase().mimeTypeForFile(oobUrl.fileName()).name();

                if(! askBeforeDownloading_)
                    downloadManager_->doDownload(oobUrl, messageId); // keep the fragment in the sent message
            }
        }

        bool isGroupMessage = false;
        if (message.type() == QXmppMessage::GroupChat)
        {
            isGroupMessage = true;
        }

        if (messageId.isEmpty() == true)
        {
            // No message id, try xep 0359
            messageId = message.stanzaId();

            // still empty?
            if (messageId.isEmpty() == true)
            {
                messageId = QString::number(QDateTime::currentMSecsSinceEpoch());
            }
        }

        if (!sentCarbon)
        {
            persistence_->addMessage(messageId,
                                     QXmppUtils::jidToBareJid(message.from()),
                                     QXmppUtils::jidToResource(message.from()),
                                     message.body(), type, 1, security);
        } else
        {
            persistence_->addMessage(messageId,
                                     QXmppUtils::jidToBareJid(message.to()),
                                     QXmppUtils::jidToResource(message.to()),
                                     message.body(), type, 0, security);
        }

        // xep 0333
        QString currentChatPartner = persistence_->getCurrentChatPartner();
        //qDebug() << "fromJid: " << message.from() << "current: " << currentChatPartner << ", isGroup: " << isGroupMessage << ", appActive? " << appIsActive_;
        if ( (currentChatPartner.compare(QXmppUtils::jidToBareJid(message.from()), Qt::CaseInsensitive) == 0) &&     // immediatelly send read notification if sender is current chat partner
             (appIsActive_ == true)                                                                                  // but only if app is active
             )
        {
            this->sendDisplayedForJid(currentChatPartner);
        }
    }
}

void MessageHandler::sendMessage(QString const &toJid, QString const &message, QString const &type, bool isGroup)
{
    QXmppMessage msg("", toJid, message);
    unsigned int security = 0;

    msg.setReceiptRequested(true);
    msg.setMarkable(true);

    // exchange body by omemo stuff if applicable
    if ((settings_->getSoftwareFeatureOmemoEnabled() == true)
        && (! settings_->getSendPlainText().contains(toJid))) // no force for plain text msg in settings
    {
        msg.setEncryptionMethod(QXmpp::Encryption::OMEMO);
        security = 1;
    }
    else // xep-0066. Only add the stanza on plain-text messages, as described in the xep-0454
    {
        if(type.compare("txt", Qt::CaseInsensitive) != 0)   // XEP-0066
        {
            msg.setOutOfBandUrl(message);
        }
    }

    if(isGroup == true)
    {
        msg.setType(QXmppMessage::GroupChat);
    }

    //qDebug() << "sendMessage id:" << msg.id() << " body:" << message << endl;

    qXmppClient_->sendPacket(msg);

    persistence_->addMessage( msg.id(),
                              QXmppUtils::jidToBareJid(msg.to()),
                              QXmppUtils::jidToResource(msg.to()),
                              message, type, 0, security);

    emit messageSent(msg.id());
}

void MessageHandler::sendDisplayedForJid(const QString &jid)
{
    if(settings_->getSendReadNotifications())
    {
        chatMarkers_->sendDisplayedForJid(jid);
    }
}

void MessageHandler::downloadFile(const QString &str, const QString &msgId)
{
    downloadManager_->doDownload(QUrl(str), msgId);
}

void MessageHandler::slotAppGetsActive(bool active)
{
    appIsActive_ = active;
}

void MessageHandler::setAskBeforeDownloading(bool AskBeforeDownloading)
{
    askBeforeDownloading_ = AskBeforeDownloading;
}
