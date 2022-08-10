#include "JingleICETransportMethodPayloadParser.h"

#include <Swiften/Swiften.h>

#include <QDebug>

JingleICETransportMethodPayloadParser::JingleICETransportMethodPayloadParser() : level_(0) {

}

void JingleICETransportMethodPayloadParser::handleStartElement(const std::string& element, const std::string&, const Swift::AttributeMap& attributes) {

    qDebug() << "JingleICETransportMethodPayloadParser::handleStartElement" << endl;


    if (level_ == 0) {
        getPayloadInternal()->setPwd(attributes.getAttributeValue("pwd").get_value_or(""));
        getPayloadInternal()->setUFrag(attributes.getAttributeValue("ufrag").get_value_or(""));
    } else if (level_ == 1) {
        if (element == "candidate") {
            JingleICETransportPayload::Candidate candidate;
            candidate.id = attributes.getAttributeValue("id").get_value_or("");
            candidate.port = attributes.getAttributeValue("port").get_value_or("");
            candidate.ip = attributes.getAttributeValue("ip").get_value_or("");
            candidate.network = attributes.getAttributeValue("network").get_value_or("");
            candidate.type = attributes.getAttributeValue("type").get_value_or("");
            candidate.protocol = attributes.getAttributeValue("protocol").get_value_or("");

            int priority = -1;
            try {
                priority = boost::lexical_cast<int>(attributes.getAttributeValue("priority").get_value_or("-1"));
            } catch(boost::bad_lexical_cast &) { }
            candidate.priority = priority;

            getPayloadInternal()->addCandidate(candidate);
        } 
    }

    ++level_;
}

void JingleICETransportMethodPayloadParser::handleEndElement(const std::string&, const std::string&) {
    --level_;
}

void JingleICETransportMethodPayloadParser::handleCharacterData(const std::string&) {

}

