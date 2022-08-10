#pragma once

#include "JingleICETransportMethodPayloadParser.h"

#include <Swiften/Swiften.h>

class JingleICETransportMethodPayloadParserFactory : public Swift::PayloadParserFactory {
    public:
        JingleICETransportMethodPayloadParserFactory() {
        }

        virtual bool canParse(const std::string& element, const std::string& ns, const Swift::AttributeMap&) const {
            return element == "transport" && ns == "urn:xmpp:jingle:transports:ice-udp:1";
        }

        virtual Swift::PayloadParser* createPayloadParser() {
            return new JingleICETransportMethodPayloadParser();
        }
};
