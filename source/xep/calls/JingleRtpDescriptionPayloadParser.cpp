#include "JingleRtpDescriptionPayloadParser.h"

#include <QDebug>

JingleRtpDescriptionPayloadParser::JingleRtpDescriptionPayloadParser() :   
    level_(DescriptionLevel), currentPayloadType_(nullptr) {
}

void JingleRtpDescriptionPayloadParser::handleStartElement(const std::string& element, const std::string&, const Swift::AttributeMap& attributes) {
    currentText_.clear();

    qDebug() << "JingleRtpDescriptionPayloadParser::handleStartElement" << endl;
    
    if (level_ == DescriptionLevel && element == "description") {
        getPayloadInternal()->setMedia(attributes.getAttribute("media"));
        currentPayloadType_  = nullptr;
    }
    else if(level_ == PayloadTypeLevel)
    {
        if(element == "payload-type")
        {    
            currentPayloadType_ = std::make_shared<JingleRtpPayloadType>(std::stoi(attributes.getAttribute("id")), 
                    attributes.getAttribute("name"), attributes.getAttribute("clockrate"), 
                    attributes.getAttribute("channels"));
            getPayloadInternal()->addPayloadType(currentPayloadType_);
        }
    }
    else if(level_ == ParameterLevel && element == "parameter")
    {
        if(currentPayloadType_ != nullptr)
        {
            std::string name = attributes.getAttribute("name");
            std::string value = attributes.getAttribute("value");

            if(name.size() > 0)
                currentPayloadType_->addParameter(name, value);
        }
    }

    ++level_;
}

void JingleRtpDescriptionPayloadParser::handleEndElement(const std::string& element, const std::string& ns) {
    --level_;

    if(level_ == PayloadTypeLevel)
    {
        if(element == "bandwidth")
        {
            getPayloadInternal()->setBandwidth(currentText_);
        }
    }
}

void JingleRtpDescriptionPayloadParser::handleCharacterData(const std::string& data) {
    currentText_ += data;
}
