#include "IncomingCallManager.h"
#include "JingleRtpDescription.h"
#include "JingleICETransportMethodPayload.h"

#include <QDebug>

#include <Swiften/Swiften.h>

IncomingCallManager::IncomingCallManager(Swift::JingleSessionManager* jingleSessionManager) :
    jingleSessionManager_(jingleSessionManager)
{
    jingleSessionManager_->addIncomingSessionHandler(this);
}


IncomingCallManager::~IncomingCallManager() {
    jingleSessionManager_->removeIncomingSessionHandler(this);
}   

bool IncomingCallManager::handleIncomingJingleSession(
        Swift::JingleSession::ref session,
        const std::vector<Swift::JingleContentPayload::ref>& contents,
        const Swift::JID& recipient) {

    qDebug() << "=====> INCOMING SESSION " << endl;

    if (Swift::JingleContentPayload::ref content = Swift::Jingle::getContentWithDescription<JingleRtpDescription>(contents)) {
        qDebug() << "=====> JINGLE RTP DESCRIPTION " << endl;
        if (content->getTransport<JingleICETransportPayload>() ) {
            qDebug() << "=====> ICE TRANSPORT " << endl;
            JingleRtpDescription::ref description = content->getDescription<JingleRtpDescription>();
            if (description) {
                JingleICETransportPayload::ref transfer = std::make_shared<JingleICETransportPayload>();
                onIncomingCall(description, transfer);
            }
            else {
                session->sendTerminate(Swift::JinglePayload::Reason::FailedApplication);
            }
        }
        else {
            session->sendTerminate(Swift::JinglePayload::Reason::UnsupportedTransports);
        }
        return true;
    }
    else {
        return false;
    }
}

void IncomingCallManager::onIncomingCall(JingleRtpDescription::ref description, JingleICETransportPayload::ref transport)
{
    qDebug() << "Call received: Pwd:" << QString::fromStdString(transport->getPwd()) << endl;  

    qDebug() << " Media:" << QString::fromStdString(description->getMedia()) << endl;    

    for(int i=0; i<description->getPayloadTypes().size(); i++)
    {
        std::shared_ptr<JingleRtpPayloadType> p = description->getPayloadTypes().at(i);

        qDebug() << " Playload[" << i << "]"
                 << ": id:" << p->getId() 
                 << ", name: " << QString::fromStdString(p->getName()) 
                 << ", clockrate:" << QString::fromStdString(p->getClockRate()) 
                 << endl;                  
    }    

    qDebug() << " Pwd:" << QString::fromStdString(transport->getPwd()) << endl;    
    qDebug() << " Ufrag:" << QString::fromStdString(transport->getUFrag()) << endl;    
    qDebug() << " Candidates: " << transport->getCandidates().size() << endl;    
    for ( int i = 0; i<transport->getCandidates().size() ; i++ )
    {
        const JingleICETransportPayload::Candidate &c = (transport->getCandidates())[i];
        qDebug() << "Candidate[" << i << "]: " 
                 << " id:" << QString::fromStdString(c.id) 
                 << " foundation: " << QString::fromStdString(c.foundation) 
                 << " port:" << QString::fromStdString(c.port)
                 << " ip:" << QString::fromStdString(c.ip)
                 << " network:" << QString::fromStdString(c.network)
                 << " protocol:" << QString::fromStdString(c.protocol)
                 << " type:" << QString::fromStdString(c.type)
                 << endl;    
    }
}
