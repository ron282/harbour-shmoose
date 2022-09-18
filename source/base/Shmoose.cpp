#include "Shmoose.h"

#include <iostream>
#include <boost/bind.hpp>

#include <QtConcurrent>
#include <QDateTime>
#include <QUrl>
#include <QMimeDatabase>

#include <QTimer>

#include <QDebug>

#include <Swiften/Elements/DiscoInfo.h>
#include <Swiften/Elements/DiscoItems.h>
#include <Swiften/Queries/Requests/EnableCarbonsRequest.h>

#include <Swiften/Base/IDGenerator.h>

#include "QXmppLogger.h"
#include "QXmppUtils.h"
#include "QXmppMessage.h"

#include "RosterController.h"
#include "Persistence.h"
#include "MessageController.h"

#include "ConnectionHandler.h"
#include "MessageHandler.h"
#include "HttpFileUploadManager.h"
#include "MamManager.h"
#include "MucManager.h"
#include "DiscoInfoHandler.h"
#include "CryptoHelper.h"
#include "StanzaId.h"
#include "Settings.h"


#include "System.h"

Shmoose::Shmoose(Swift::NetworkFactories* networkFactories, QObject *parent) :
    QObject(parent),
    netFactories_(networkFactories),
    rosterController_(new RosterController(this)),
    persistence_(new Persistence(this)),
    settings_(new Settings(this)),
    stanzaId_(new StanzaId(this)),
    connectionHandler_(new ConnectionHandler(this)),
    messageHandler_(new MessageHandler(persistence_, settings_, rosterController_, this)),
    httpFileUploadManager_(new HttpFileUploadManager(this)),
    mamManager_(new MamManager(persistence_, this)),
    mucManager_(new MucManager(this)),
    discoInfoHandler_(new DiscoInfoHandler(httpFileUploadManager_, mamManager_, this)),
    jid_(""), password_(""),
    version_("0.8.0"),
    notSentMsgId_(""),
    qXmppClient_(new QXmppClient(this))
{
    qApp->setApplicationVersion(version_);


    qXmppClient_->connect(qXmppClient_, &QXmppClient::connected, this, &Shmoose::onConnected);

    connect(connectionHandler_, SIGNAL(signalInitialConnectionEstablished()), this, SLOT(intialSetupOnFirstConnection()));

    connect(httpFileUploadManager_, SIGNAL(fileUploadedForJidToUrl(QString,QString,QString)),
            this, SLOT(fileUploaded(QString,QString,QString)));
    connect(httpFileUploadManager_, SIGNAL(fileUploadFailedForJidToUrl()), 
            this, SLOT(attachmentUploadFailed()));

    connect(mucManager_, SIGNAL(newGroupForContactsList(QString,QString)), rosterController_, SLOT(addGroupAsContact(QString,QString)));
    connect(mucManager_, SIGNAL(removeGroupFromContactsList(QString)), rosterController_, SLOT(removeGroupFromContacts(QString)) );

    connect(discoInfoHandler_, SIGNAL(serverHasHttpUpload_(bool)), this, SIGNAL(signalCanSendFile(bool)));
    connect(discoInfoHandler_, SIGNAL(serverHasMam_(bool)), mamManager_, SLOT(setServerHasFeatureMam(bool)));
    connect(mucManager_, SIGNAL(newGroupForContactsList(QString,QString)), mamManager_, SLOT(receiveRoomWithName(QString, QString)));

    // send read notification if app gets active
    connect(this, SIGNAL(signalAppGetsActive(bool)), this, SLOT(sendReadNotification(bool)));

    // inform connectionHandler and messageHandler about app status
    connect(this, SIGNAL(signalAppGetsActive(bool)), connectionHandler_, SLOT(slotAppGetsActive(bool)));
    connect(this, SIGNAL(signalAppGetsActive(bool)), messageHandler_, SLOT(slotAppGetsActive(bool)));

    // proxy signal to qml ui
    connect(connectionHandler_, SIGNAL(signalHasInetConnection(bool)), this, SIGNAL(signalHasInetConnection(bool)));
    connect(connectionHandler_, SIGNAL(connectionStateChanged()), this, SIGNAL(connectionStateChanged()));

    // show errors to user
    connect(mucManager_, SIGNAL(signalShowMessage(QString,QString)), this, SIGNAL(signalShowMessage(QString,QString)));
    connect(rosterController_, SIGNAL(signalShowMessage(QString,QString)), this, SIGNAL(signalShowMessage(QString,QString)));

    // show status to user
    connect(httpFileUploadManager_, SIGNAL(showStatus(QString, QString)), this, SIGNAL(signalShowStatus(QString, QString)));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(slotAboutToQuit()));

    connect(settings_, SIGNAL(compressImagesChanged(bool)), httpFileUploadManager_, SLOT(setCompressImages(bool)));
    connect(settings_, SIGNAL(limitCompressionChanged(unsigned int)), httpFileUploadManager_, SLOT(setLimitCompression(unsigned int)));

    connect(messageHandler_, SIGNAL(httpDownloadFinished(QString)), this, SIGNAL(httpDownloadFinished(QString)));
}

Shmoose::~Shmoose()
{
    qDebug() << "Shmoose::~Shmoose";

    if (connectionHandler_->isConnected())
    {
        softwareVersionResponder_->stop();

        if( tracer_ != nullptr) delete tracer_;
        delete softwareVersionResponder_;
        delete client_;
    }
}

void Shmoose::slotAboutToQuit()
{
    if (connectionHandler_->isConnected())
    {
        client_->disconnect();
    }
}

void Shmoose::mainConnect(const QString &jid, const QString &pass)
{
    persistence_->openDatabaseForJid(jid);

    QString resourceName;

    resourceName = QString("shmoose.") + System::getUniqueResourceId();
    QString completeJid = jid + "/" + resourceName;

#ifndef SFOS
    completeJid += "Desktop";
    setHasInetConnection(true);
#endif

    // setup the xmpp client
    client_ = new Swift::Client(Swift::JID(completeJid.toStdString()), pass.toStdString(), netFactories_);
    client_->setAlwaysTrustCertificates();

    qXmppClient_->connectToServer(completeJid+".new", pass);
    rosterController_->setupWithClient(qXmppClient_);
    stanzaId_->setupWithClient(client_);
    connectionHandler_->setupWithClient(client_);
    messageHandler_->setupWithClient(qXmppClient_);

    //tracer_ = new Swift::ClientXMLTracer(client_);
    tracer_ = nullptr;
    qXmppClient_->logger()->setLoggingType(QXmppLogger::StdoutLogging);

    // configure the xmpp client
    softwareVersionResponder_ = new Swift::SoftwareVersionResponder(client_->getIQRouter());
    softwareVersionResponder_->setVersion("Shmoose", version_.toStdString());
    softwareVersionResponder_->start();
    client_->setSoftwareVersion("Shmoose", version_.toStdString());

    // register capabilities
    Swift::DiscoInfo discoInfo;
    discoInfo.addIdentity(Swift::DiscoInfo::Identity("shmoose", "client", "phone"));

    // http://xmpp.org/extensions/xep-0184.html, MessageDeliveryReceiptsFeature
    discoInfo.addFeature(Swift::DiscoInfo::MessageDeliveryReceiptsFeature);

    // https://xmpp.org/extensions/xep-0333.html
    discoInfo.addFeature("urn:xmpp:chat-markers:0");

    // https://xmpp.org/extensions/xep-0280.html
    discoInfo.addFeature(Swift::DiscoInfo::MessageCarbonsFeature);

    // omemo
    Settings settings;
    if (settings.getSoftwareFeatureOmemoEnabled() == true)
    {
//        discoInfo.addFeature(lurchAdapter_->getFeature().toStdString());
//        discoInfo.addFeature(lurchAdapter_->getFeature().toStdString() + "+notify");
    }

    // identify myself
    client_->getDiscoManager()->setCapsNode("https://github.com/geobra/harbour-shmoose");

    // setup this disco info
    client_->getDiscoManager()->setDiscoInfo(discoInfo);

    // finaly try to connect
    client_->connect();

    // for saving on connection success
    if (settings_->getSaveCredentials() == true)
    {
        jid_ = jid;
        password_ = pass;
    }

    httpFileUploadManager_->setCompressImages(settings_->getCompressImages());
    httpFileUploadManager_->setLimitCompression(settings_->getLimitCompression());
}

void Shmoose::mainDisconnect()
{
    if (connectionState())

    {
        client_->disconnect();
    }
}

void Shmoose::reConnect()
{
    if (client_ != nullptr)
    {
        client_->connect();
    }
}

void Shmoose::intialSetupOnFirstConnection()
{
    // pass the client pointer to the httpFileUploadManager
    httpFileUploadManager_->setupWithClient(client_);

    // init mam
    mamManager_->setupWithClient(client_);

    // init and setup discoInfoHandler
    discoInfoHandler_->setupWithClient(client_);

    // init and setup mucManager
    mucManager_->setupWithClient(client_);

    // init and setup omemo stuff
    if (settings_->getSoftwareFeatureOmemoEnabled() == true)
    {
//        lurchAdapter_->setupWithClient(client_);
    }

    // Save account data
    settings_->setJid(jid_);
    settings_->setPassword(password_);
}

void Shmoose::onConnected()
{
}

void Shmoose::setCurrentChatPartner(QString const &jid)
{
    persistence_->setCurrentChatPartner(jid);

    sendReadNotification(true);
}

QString Shmoose::getCurrentChatPartner()
{
    return persistence_->getCurrentChatPartner();
}

void Shmoose::sendMessage(QString const &toJid, QString const &message, QString const &type)
{
    bool isGroup = rosterController_->isGroup(toJid);
    messageHandler_->sendMessage(toJid, message, type, isGroup);
}

void Shmoose::sendMessage(QString const &message, QString const &type)
{
    const QString toJid = getCurrentChatPartner();

    if (! toJid.isEmpty())
    {
        bool isGroup = rosterController_->isGroup(toJid);
        messageHandler_->sendMessage(toJid, message, type, isGroup);
    }
    else
    {
        qDebug() << "tried to send msg without current chat partner selected!";
    }
}


void Shmoose::sendFile(QString const &toJid, QString const &file)
{
    bool shouldEncryptFile = settings_->getSoftwareFeatureOmemoEnabled() && (! settings_->getSendPlainText().contains(toJid));

    QXmppMessage msg("", toJid, file);

    // messsage is added to the database 
    persistence_->addMessage(msg.id(),
                             QXmppUtils::jidToBareJid(toJid),
                             QXmppUtils::jidToResource(toJid),
                             file,
                             QMimeDatabase().mimeTypeForFile(file).name(), 0, shouldEncryptFile ? 1 : 0);

    persistence_->markMessageAsUploadingAttachment(msg.id());

    bool success = httpFileUploadManager_->requestToUploadFileForJid(file, toJid, shouldEncryptFile);

    if(!success)
    {
        persistence_->markMessageAsSendFailed(msg.id());
    }
}

void Shmoose::sendFile(QUrl const &file)
{
    const QString toJid = getCurrentChatPartner();
    QString localFile = file.toLocalFile();

    sendFile(toJid, localFile);
}


void Shmoose::sendReadNotification(bool active)
{
    QString currentChatPartner = persistence_->getCurrentChatPartner();

    if (active == true && (! currentChatPartner.isEmpty()))
    {
        messageHandler_->sendDisplayedForJid(currentChatPartner);
    }
}

RosterController* Shmoose::getRosterController()
{
    return rosterController_;
}

Persistence* Shmoose::getPersistence()
{
    return persistence_;
}

Settings* Shmoose::getSettings()
{
    return settings_;
}

bool Shmoose::connectionState() const
{
    return connectionHandler_->isConnected();
}

bool Shmoose::canSendFile()
{
    return httpFileUploadManager_->getServerHasFeatureHttpUpload();
}

bool Shmoose::isOmemoUser(const QString& jid)
{
//    return lurchAdapter_->isOmemoUser(jid);
    return true;
}

QString Shmoose::getAttachmentPath()
{
    return System::getAttachmentPath();
}

QString Shmoose::getLocalFileForUrl(const QString& str)
{
    QUrl url(str);

    if(url.isRelative())
    {
        return str;
    }
    else
    {
        QString localFile;

        localFile = System::getAttachmentPath() + QDir::separator() + CryptoHelper::getHashOfString(url.toString(), true);
        if(QFile::exists(localFile) && QFileInfo(localFile).size() > 0)
            return localFile;
        else
            return "";
    }
}

void Shmoose::downloadFile(const QString& str, const QString& msgId)
{
    messageHandler_->downloadFile(str, msgId);
}

void Shmoose::setHasInetConnection(bool connected)
{
    connectionHandler_->setHasInetConnection(connected);
}

void Shmoose::setAppIsActive(bool active)
{
    emit signalAppGetsActive(active);
}

QString Shmoose::getVersion()
{
    return version_;
}

void Shmoose::joinRoom(QString const &roomJid, QString const &roomName)
{
    Swift::JID jid(roomJid.toStdString());

    if (jid.isValid())
    {
        setCurrentChatPartner(roomJid); // this prevents notifications for each initial history message
        mucManager_->addRoom(jid, roomName);
    }
    else
    {
        emit signalShowMessage("Join room", "JID not valid!");
    }
}

void Shmoose::removeRoom(QString const &roomJid)
{
    mucManager_->removeRoom(roomJid);
}

void Shmoose::attachmentUploadFailed()
{
    persistence_->markMessageAsSendFailed(notSentMsgId_);
    notSentMsgId_ = "";
}

void Shmoose::saveAttachment(const QString& msg)
{
    //TODO Error management + Destination selection
    QFile::copy(getLocalFileForUrl(msg), QStandardPaths::locate(QStandardPaths::DownloadLocation, "", QStandardPaths::LocateDirectory)  +
                    QDir::separator() + CryptoHelper::getHashOfString(QUrl(msg).toString(), true));
}

void Shmoose::fileUploaded(QString const&toJid, QString const&message, QString const&type)
{
    persistence_->removeMessage(notSentMsgId_, toJid);
    notSentMsgId_ = "";
    sendMessage(toJid, message, type);
}

unsigned int Shmoose::getMaxUploadSize()
{
    return httpFileUploadManager_->getMaxFileSize();
}
