
#pragma once

#include "JingleRtpDescription.h"
#include "JingleRtpPayloadType.h"

#include <Swiften/Swiften.h>

class JingleRtpDescriptionPayloadParser : public Swift::GenericPayloadParser<JingleRtpDescription> {
    public:
        JingleRtpDescriptionPayloadParser();

        virtual void handleStartElement(const std::string& element, const std::string&, const Swift::AttributeMap& attributes);
        virtual void handleEndElement(const std::string& element, const std::string&);
        virtual void handleCharacterData(const std::string& data);

    private:
        enum Level {
            DescriptionLevel = 0,
            PayloadTypeLevel = 1,
            ParameterLevel   = 2
        };
        int level_;
        std::shared_ptr<JingleRtpPayloadType> currentPayloadType_;
        std::string currentText_;
};

