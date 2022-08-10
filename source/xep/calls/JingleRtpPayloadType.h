#pragma once

#include <Swiften/Swiften.h>

class  JingleRtpPayloadType : public Swift::Payload {
    typedef std::shared_ptr<JingleRtpPayloadType> ref;

    public:
        JingleRtpPayloadType(int id, const std::string& name = "", 
                             const std::string& clockRate = "", 
                             const std::string& channels = "", 
                             const boost::posix_time::ptime &maxPTime = boost::posix_time::ptime(), 
                             const boost::posix_time::ptime &pTime = boost::posix_time::ptime()) :
            id_(id), name_(name), clockRate_(clockRate), channels_(channels), pTime_(pTime), maxPTime_(maxPTime)  {
        }

    public:
        typedef std::map<std::string, std::string> ParametersMap;

    public:
        void setId(int id) {
            id_ = id;
        }

        int getId() const {
            return id_;
        }

        void setName(const std::string& name) {
            name_ = name;;
        }

        const std::string& getName() const {
            return name_;
        }

        void setClockRate(const std::string& clockRate) {
            clockRate_ = clockRate;
        }

        const std::string& getClockRate() const {
            return clockRate_;
        }

        void setChannels(const std::string& channels) {
            channels_ = channels;
        }

        const std::string& getChannels() const {
            return channels_;
        }

        void setMaxPTime(const boost::posix_time::ptime maxPTime) {
            maxPTime_ = maxPTime;
        }

        boost::posix_time::ptime getMaxPTime() const {
            return maxPTime_;
        }

        void setPTime(const boost::posix_time::ptime pTime) {
            pTime_ = pTime;
        }

        boost::posix_time::ptime getPTime() const {
            return pTime_;
        }

        void addParameter(const std::string &name, const std::string &value)
        {
            parameters_[name] = value;
        }

        const std::map<std::string, std::string> &getParameters()
        {
            return parameters_;
        }

    private:
        int id_;
        std::string name_;
        std::string clockRate_;
        std::string channels_;
        boost::posix_time::ptime pTime_;
        boost::posix_time::ptime maxPTime_;
        ParametersMap parameters_;
};
