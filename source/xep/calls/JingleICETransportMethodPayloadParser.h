#pragma once

#include "JingleICETransportMethodPayload.h"
#include <Swiften/Swiften.h>

class JingleICETransportMethodPayloadParser : public Swift::GenericPayloadParser<JingleICETransportPayload> {
    public:
        JingleICETransportMethodPayloadParser();

        virtual void handleStartElement(const std::string& element, const std::string&, const Swift::AttributeMap& attributes);
        virtual void handleEndElement(const std::string& element, const std::string&);
        virtual void handleCharacterData(const std::string& data);

    private:
        int level_;
};

