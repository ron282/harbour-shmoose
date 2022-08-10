#pragma once

#include <Swiften/Swiften.h>


class JingleICETransportPayload : public Swift::JingleTransportPayload {
    public:
        struct Candidate {

            Candidate() : priority(0), type("udp") {}

            std::string id;
            std::string foundation;
            std::string port;
            std::string ip;
            std::string network;
            std::string protocol;
            std::string type;
            int priority;
        };

        struct CompareCandidate {
            bool operator() (const JingleICETransportPayload::Candidate& c1, const JingleICETransportPayload::Candidate& c2) const {
                if (c1.priority < c2.priority) return true;
                return false;
            }
        };

    public:
        JingleICETransportPayload() : candidateError_(false) {}

        const std::vector<Candidate>& getCandidates() const {
            return candidates;
        }

        void addCandidate(const Candidate& candidate) {
            candidates.push_back(candidate);
        }

        void setCandidateUsed(const std::string& id) {
            candidateUsedID_ = id;
        }

        const std::string& getCandidateUsed() const {
            return candidateUsedID_;
        }

        void setActivated(const std::string& id) {
            activatedID_ = id;
        }

        const std::string& getActivated() const {
            return activatedID_;
        }

        void setCandidateError(bool hasError) {
            candidateError_ = hasError;
        }

        bool hasCandidateError() const {
            return candidateError_;
        }

        void setPwd(const std::string& pwd) {
            pwd_= pwd;
        }

        const std::string& getPwd() const {
            return pwd_;
        }

        void setUFrag(const std::string& ufrag) {
            ufrag_= ufrag;
        }

        const std::string& getUFrag() const {
            return ufrag_;
        }

    public:
        typedef std::shared_ptr<JingleICETransportPayload> ref;

    private:
        std::vector<Candidate> candidates;

        std::string candidateUsedID_;
        std::string activatedID_;
        std::string pwd_;
        std::string ufrag_;
        bool candidateError_;
};
