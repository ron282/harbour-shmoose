#pragma once

#include <QObject>

#include <Swiften/Swiften.h>

class CallsManager : public QObject
{
    Q_OBJECT
public:
    explicit CallsManager(QObject *parent = nullptr);
    void setupWithClient(Swift::Client* client);

private slots:
    void handleConnected();

private:
    Swift::Client* client_;
    Swift::JingleSessionManager* jingleSessionManager_; 
};
