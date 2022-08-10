#pragma once

#include "JingleRtpPayloadType.h"

#include <Swiften/Swiften.h>


class  JingleRtpDescription : public Swift::JingleDescription {
    public:
        typedef std::shared_ptr<JingleRtpDescription> ref; 

        const std::vector<std::shared_ptr<JingleRtpPayloadType> > getPayloadTypes() {
            return payloadTypes_;
        }

        void addPayloadType(std::shared_ptr<JingleRtpPayloadType> payloadType) {
            payloadTypes_.push_back(payloadType);
        }

        void setMedia(const std::string &media) {
            media_ = media;
        }

        const std::string &getMedia() const
        {
            return media_;
        }

        void setBandwidth(const std::string &bandwidth) {
            bandwidth_ = bandwidth;
        }

        const std::string &getBandwidth() const
        {
            return bandwidth_;
        }

    private:
        std::vector<std::shared_ptr<JingleRtpPayloadType> > payloadTypes_;
        std::string media_;
        std::string bandwidth_;
};
