#pragma once

#include "JingleRtpDescription.h"
#include "JingleICETransportMethodPayload.h"

#include <Swiften/Swiften.h>

class IncomingCallManager : public Swift::IncomingJingleSessionHandler
{
public:
    IncomingCallManager(Swift::JingleSessionManager* jingleSessionManager);
    ~IncomingCallManager();
    virtual bool handleIncomingJingleSession(Swift::JingleSession::ref, const std::vector<Swift::JingleContentPayload::ref>& contents, const Swift::JID& recipient);
    virtual void onIncomingCall(JingleRtpDescription::ref, JingleICETransportPayload::ref);

protected: 
    Swift::JingleSessionManager* jingleSessionManager_;
};
