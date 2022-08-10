#pragma once

#include "JingleRtpDescriptionPayloadParser.h"

#include <Swiften/Swiften.h>

class JingleRtpDescriptionPayloadParserFactory : public Swift::PayloadParserFactory {
    public:
        JingleRtpDescriptionPayloadParserFactory() {
        }

        virtual bool canParse(const std::string& element, const std::string& ns, const Swift::AttributeMap&) const {
            return element == "description" && ns == "urn:xmpp:jingle:apps:rtp:1";
        }

        virtual Swift::PayloadParser* createPayloadParser() {
            return new JingleRtpDescriptionPayloadParser();
        }
};


