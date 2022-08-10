#include "CallsManager.h"
#include "IncomingCallManager.h"
#include "JingleRtpDescriptionPayloadParserFactory.h"
#include "JingleICETransportMethodPayloadParserFactory.h"

#include <Swiften/Swiften.h>
#include <Swiften/Base/Log.h>


CallsManager::CallsManager(QObject *parent) : QObject(parent),
    client_(nullptr), jingleSessionManager_(nullptr)
{
}

void CallsManager::setupWithClient(Swift::Client* client)
{
    Swift::Log::setLogLevel(Swift::Log::debug);

    client_ = client;

    client_->addPayloadParserFactory(new JingleRtpDescriptionPayloadParserFactory());
    client_->addPayloadParserFactory(new JingleICETransportMethodPayloadParserFactory());

    jingleSessionManager_ = new Swift::JingleSessionManager(client_->getIQRouter());
    new IncomingCallManager (jingleSessionManager_);

    client_->onConnected.connect(boost::bind(&CallsManager::handleConnected, this));
}

void CallsManager::handleConnected() {
    

    //client->getFileTransferManager()->onIncomingFileTransfer.connect(boost::bind(&FileReceiver::handleIncomingFileTransfer, this, _1));

    /*
    DiscoInfo discoInfo;
    discoInfo.addIdentity(DiscoInfo::Identity(CLIENT_NAME, "client", "pc"));
    discoInfo.addFeature(DiscoInfo::JingleFeature);
    discoInfo.addFeature(DiscoInfo::JingleFTFeature);
    discoInfo.addFeature(DiscoInfo::Bytestream);
    discoInfo.addFeature(DiscoInfo::JingleTransportsIBBFeature);
    discoInfo.addFeature(DiscoInfo::JingleTransportsS5BFeature);
    client->getDiscoManager()->setCapsNode(CLIENT_NODE);
    client->getDiscoManager()->setDiscoInfo(discoInfo);
    client->getPresenceSender()->sendPresence(Presence::create());
    */
}

