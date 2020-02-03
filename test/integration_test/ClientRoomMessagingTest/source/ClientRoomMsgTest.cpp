#include "ClientRoomMsgTest.h"
#include "DbusInterfaceWrapper.h"

#include <QSignalSpy>
#include <QImageWriter>
#include <QDebug>

/*
 * Common Tests:
 * Connect, Disconnect, Reconnect
 *
 * Group:
 * Join group, leave group
 * send group msg, check status. Send group msg with offline clients, check status. Reconnect and check status again
 * Check simple mam
 * Check recieving order of msg on reconnect
 *
 */

ClientRoomMsgTest::ClientRoomMsgTest() : ClientComTestCommon(), user3jid_("user3@localhost")
{

}

void ClientRoomMsgTest::initTestCase()
{
    ClientComTestCommon::initTestCase();

    QString dbusServiceNameCommon("org.shmoose.dbuscom");
    QString dbusServiceNameMhs = dbusServiceNameCommon + "mhs";
    QString dbusObjectPath("/client");

    interfaceMhs_ = new DbusInterfaceWrapper(dbusServiceNameMhs, dbusObjectPath, "", QDBusConnection::sessionBus(), this);
}


void ClientRoomMsgTest::sendRoomMsgTest()
{
    // The message states: (0) unknown, (1) received, (2) displayed

    // need to collect more then one signal here. qsignalspy only catches one at a time. Use an own slot to collet them all.
    QObject::connect(interfaceLhs_->getInterface(), SIGNAL(signalRoomMsgState(QString, QString, int)), this, SLOT(collectMsgStateChangedLhs(QString, QString, int)));
    QSignalSpy spyMsgStateRoomLhs(interfaceLhs_->getInterface(), SIGNAL(signalRoomMsgState(QString, QString, int)));
    QSignalSpy spyMsgStateRoomRhs(interfaceRhs_->getInterface(), SIGNAL(signalRoomMsgState(QString, QString, int)));
    QSignalSpy spyMsgStateRoomMhs(interfaceMhs_->getInterface(), SIGNAL(signalRoomMsgState(QString, QString, int)));

    const QString roomJid = "testroom@conference.localhost";

    // user1 and user2 already logged in. Login user3
    connectionTestCommon(interfaceMhs_, user3jid_, "user3");
    requestRosterTestCommon(interfaceMhs_);

    // user1: user2 (from base), user3
    addContactTestCommon(interfaceLhs_, user3jid_, "user3");

    // user2: user1 (from base), user3
    addContactTestCommon(interfaceRhs_, user3jid_, "user3");

    // user3: user1, user2
    addContactTestCommon(interfaceMhs_, user1jid_, "user1");
    addContactTestCommon(interfaceMhs_, user2jid_, "user2");

    // --------------------------
    // all clients join the room
    // --------------------------
    QSignalSpy joinRoomLhs(interfaceLhs_->getInterface(), SIGNAL(signalRoomJoined(QString, QString)));
    QSignalSpy joinRoomMhs(interfaceMhs_->getInterface(), SIGNAL(signalRoomJoined(QString, QString)));
    QSignalSpy joinRoomRhs(interfaceRhs_->getInterface(), SIGNAL(signalRoomJoined(QString, QString)));

    QList<QVariant> argumentsJoinRoom {roomJid, "testroom"};
    interfaceLhs_->callDbusMethodWithArgument("joinRoom", argumentsJoinRoom);
    joinRoomLhs.wait(timeOut_);
    qDebug() << "join room count: " << joinRoomLhs.count();
#ifdef TRAVIS
    QVERIFY(joinRoomLhs.count() == 1);
    joinRoomMhs.clear();
#endif

    interfaceMhs_->callDbusMethodWithArgument("joinRoom", argumentsJoinRoom);
    joinRoomMhs.wait(timeOut_);
    qDebug() << "join room count: " << joinRoomMhs.count();
#ifdef TRAVIS
    QVERIFY(joinRoomMhs.count() == 1);
    joinRoomMhs.clear();
#endif

    interfaceRhs_->callDbusMethodWithArgument("joinRoom", argumentsJoinRoom);
    joinRoomRhs.wait(timeOut_);
    qDebug() << "join room count: " << joinRoomRhs.count();
#ifdef TRAVIS
    QVERIFY(joinRoomRhs.count() == 1);
    joinRoomRhs.clear();
#endif

    // no client had a look at any received messages at the GUI level.
    QList<QVariant> argumentsCurrentChatPartnerEmpty {""};
    interfaceLhs_->callDbusMethodWithArgument("setCurrentChatPartner", argumentsCurrentChatPartnerEmpty);
    interfaceMhs_->callDbusMethodWithArgument("setCurrentChatPartner", argumentsCurrentChatPartnerEmpty);
    interfaceRhs_->callDbusMethodWithArgument("setCurrentChatPartner", argumentsCurrentChatPartnerEmpty);

    // ---------------------------------------------------------------------
    // user1 sends msg. all other clients should receive them. check status
    // ---------------------------------------------------------------------
    const QString msgOnWireFromUser1 = "Hi room from user1";

    // setup msg text speyer
    QSignalSpy spyLatestMsgAtUser2(interfaceRhs_->getInterface(), SIGNAL(signalLatestMsg(QString, QString, QString)));
    QSignalSpy spyLatestMsgAtUser3(interfaceMhs_->getInterface(), SIGNAL(signalLatestMsg(QString, QString, QString)));

    // send the msg
    QSignalSpy spyMsgSent(interfaceLhs_->getInterface(), SIGNAL(signalMsgSent(QString)));
    QList<QVariant> argumentsMsgToRoom {roomJid, msgOnWireFromUser1};
    interfaceLhs_->callDbusMethodWithArgument("sendMsg", argumentsMsgToRoom);

    // check msgId of sent msg
    spyMsgSent.wait(timeOut_);
    QVERIFY(spyMsgSent.count() == 1);
    QList<QVariant> spyArgumentsOfMsgSent = spyMsgSent.takeFirst();
    QString msgId = spyArgumentsOfMsgSent.at(0).toString();
    qDebug() << "sent MsgId: " << msgId;

    // wait for arrived msgOnWireFromUser1 at other clients
    spyLatestMsgAtUser2.wait(timeOut_);
    QCOMPARE(spyLatestMsgAtUser2.count(), 1);

    spyLatestMsgAtUser3.wait(timeOut_);
    QCOMPARE(spyLatestMsgAtUser3.count(), 1);

    // check the msg content
    QList<QVariant> spyArgumentsOfMsgAtUser2 = spyLatestMsgAtUser2.takeFirst();
    QVERIFY(spyArgumentsOfMsgAtUser2.at(2).toString() == msgOnWireFromUser1);

    QList<QVariant> spyArgumentsOfMsgAtUser3 = spyLatestMsgAtUser3.takeFirst();
    QVERIFY(spyArgumentsOfMsgAtUser3.at(2).toString() == msgOnWireFromUser1);

    // check the msg status as seen from the sender (user1)
    spyMsgStateRoomLhs.wait(timeOut_);
    auto receivedCount = 0;
    while (! stateChangeMsgLhsList_.isEmpty())
    {
        MsgIdJidState mij = stateChangeMsgLhsList_.takeFirst();

        //qDebug() << "id: " << mij.msgId << ", jid: " << mij.jid << ", state: " << mij.state;
        if (mij.msgId.compare(msgId) != 0) // only interessted in answers to our sent msg
        {
            //qDebug() << "   ... skip!!!!";
            continue;
        }

        if (mij.jid.contains("user1") || mij.jid.contains("user2") || mij.jid.contains("user3"))
        {
            QCOMPARE(mij.state, 1); // received
            receivedCount++;
        }
    }
    QCOMPARE(receivedCount, 3);
    stateChangeMsgLhsList_.clear();

    // user2 reads msg. check msg status at user1 side for user2
    QList<QVariant> argumentsCurrentChatPartnerRoom {roomJid};
    interfaceRhs_->callDbusMethodWithArgument("setCurrentChatPartner", argumentsCurrentChatPartnerRoom);

    spyMsgStateRoomRhs.wait(timeOut_); // sends the displayed stanza
    spyMsgStateRoomLhs.wait(timeOut_); // receives the displayed stanza

    QCOMPARE(destrcutiveVerfiyStateAndCountOfMsgStates(lhs, msgId, QList<MsgIdJidState>{{"foo", "user2", 2}}), true);

    // user3 reads msg. check msg status at user1 side for user3
    interfaceMhs_->callDbusMethodWithArgument("setCurrentChatPartner", argumentsCurrentChatPartnerRoom);

    spyMsgStateRoomMhs.wait(timeOut_); // sends the displayed stanza
    spyMsgStateRoomLhs.wait(timeOut_); // receives the displayed stanza

    QCOMPARE(destrcutiveVerfiyStateAndCountOfMsgStates(lhs, msgId, QList<MsgIdJidState>{{"foo", "user3", 2}}), true);

    //---------------------------------------------------------------------------------------------------------------
    // Send group msg with offline clients, check status. Reconnect and check status again
    //---------------------------------------------------------------------------------------------------------------
    // set user2 offline
    interfaceRhs_->callDbusMethodWithArgument("setCurrentChatPartner", argumentsCurrentChatPartnerEmpty);
    interfaceRhs_->callDbusMethodWithArgument("disconnectFromServer", QList<QVariant>());

    QSignalSpy spySignalDisconnectedRhs(interfaceRhs_->getInterface(), SIGNAL(signalConnectionStateChanged()));
    spySignalDisconnectedRhs.wait(timeOutConnect_);
    QCOMPARE(spySignalDisconnectedRhs.count(), 1);

    // set user3 offline
    interfaceMhs_->callDbusMethodWithArgument("setCurrentChatPartner", argumentsCurrentChatPartnerEmpty);
    interfaceMhs_->callDbusMethodWithArgument("disconnectFromServer", QList<QVariant>());

    QSignalSpy spySignalDisconnectedMhs(interfaceMhs_->getInterface(), SIGNAL(signalConnectionStateChanged()));
    spySignalDisconnectedMhs.wait(timeOutConnect_);
    QCOMPARE(spySignalDisconnectedMhs.count(), 1);

    // send the msg
    spyMsgSent.clear();
    stateChangeMsgLhsList_.clear();
    interfaceLhs_->callDbusMethodWithArgument("sendMsg", argumentsMsgToRoom);

    // check msgId of sent msg
    spyMsgSent.wait(timeOut_);
    QVERIFY(spyMsgSent.count() == 1);
    spyArgumentsOfMsgSent = spyMsgSent.takeFirst();
    msgId = spyArgumentsOfMsgSent.at(0).toString();
    qDebug() << "sent MsgId: " << msgId;

    spyMsgStateRoomRhs.wait(timeOut_); // could send a stanza
    spyMsgStateRoomMhs.wait(timeOut_); // could send a stanza
    spyMsgStateRoomLhs.wait(timeOut_); // could receive a stanza

    // check status of msg at user1 side. No update should be there. All clients are offline! Only user1 (the sender) has virtual received his msg
    QCOMPARE(destrcutiveVerfiyStateAndCountOfMsgStates(lhs, msgId, QList<MsgIdJidState>{{"foo", "user1", 1}}), true);

    // connect the user2 client again
    interfaceRhs_->callDbusMethodWithArgument("reConnect", QList<QVariant>());

    // wait for the msg be delivered to the reconnected client (as seen from the sender)
    spyMsgStateRoomRhs.wait(timeOutConnect_);
    spyMsgStateRoomRhs.wait(timeOutConnect_);  // wait until the reconnt handshake is done

    spyMsgStateRoomLhs.wait(timeOut_); // for the msg ack stanza
    QCOMPARE(destrcutiveVerfiyStateAndCountOfMsgStates(lhs, msgId, QList<MsgIdJidState>{{"foo", "user2", 1}}), true); // received from reconnected client

    // connect the user3 client again
    interfaceMhs_->callDbusMethodWithArgument("reConnect", QList<QVariant>());

    // wait for the msg be delivered to the reconnected client (as seen from the sender)
    spyMsgStateRoomMhs.wait(timeOutConnect_);
    spyMsgStateRoomMhs.wait(timeOutConnect_);  // wait until the reconnt handshake is done

    spyMsgStateRoomMhs.wait(timeOut_); // for the msg ack stanza
    QCOMPARE(destrcutiveVerfiyStateAndCountOfMsgStates(lhs, msgId, QList<MsgIdJidState>{{"foo", "user3", 1}}), true); // received from reconnected client

    // user2 reads msg. check msg status at user1 side for user2
    interfaceRhs_->callDbusMethodWithArgument("setCurrentChatPartner", argumentsCurrentChatPartnerRoom);

    spyMsgStateRoomRhs.wait(timeOut_); // sends the displayed stanza
    spyMsgStateRoomLhs.wait(timeOut_); // receives the displayed stanza

    QCOMPARE(destrcutiveVerfiyStateAndCountOfMsgStates(lhs, msgId, QList<MsgIdJidState>{{"foo", "user2", 2}}), true);

    // user3 reads msg. check msg status at user1 side for user3
    interfaceMhs_->callDbusMethodWithArgument("setCurrentChatPartner", argumentsCurrentChatPartnerRoom);

    spyMsgStateRoomMhs.wait(timeOut_); // sends the displayed stanza
    spyMsgStateRoomLhs.wait(timeOut_); // receives the displayed stanza

    QCOMPARE(destrcutiveVerfiyStateAndCountOfMsgStates(lhs, msgId, QList<MsgIdJidState>{{"foo", "user3", 2}}), true);




    // quit clients
    interfaceLhs_->callDbusMethodWithArgument("quitClient", QList<QVariant>());
    interfaceRhs_->callDbusMethodWithArgument("quitClient", QList<QVariant>());
    interfaceMhs_->callDbusMethodWithArgument("quitClient", QList<QVariant>());
}

bool ClientRoomMsgTest::destrcutiveVerfiyStateAndCountOfMsgStates(enum side theSide, const QString& msgIdFilter, QList<MsgIdJidState> msgsList)
{
    auto returnValue = true;

    QList<MsgIdJidState> list = {};
    if (theSide == lhs)
    {
        list = stateChangeMsgLhsList_;
    }
    else if (theSide == mhs)
    {
        // TODO
    }
    else if (theSide == rhs)
    {
        // TODO
    }

    qDebug() << "before:";
    for(auto msg: list)
    {
        qDebug() << "id: " << msg.msgId << ", jid: " << msg.jid << ", state: " << msg.state;
    }

    QMutableListIterator<MsgIdJidState> i(list);
    while (i.hasNext())
    {
        if (i.next().msgId.compare(msgIdFilter) != 0)
        {
            i.remove();
        }
    }

    qDebug() << "after:";
    for(auto msg: list)
    {
        qDebug() << "id: " << msg.msgId << ", jid: " << msg.jid << ", state: " << msg.state;
    }


    if(list.count() != msgsList.count())
    {
        qDebug() << "collected list.count() is " << list.count() << " and test msgsList.count() is " << msgsList.count();
        returnValue = false;
    }

    if (returnValue == true)
    {
        int loop = 0;
        for(auto msg: list)
        {
            if (msg.state != msgsList.at(loop).state || (! msg.jid.startsWith(msgsList.at(loop).jid, Qt::CaseInsensitive)))
            {
                qDebug() << "collected msg.state is " << msg.state << " and test msgList.state is " << msgsList.at(loop).state;
                qDebug() << "collected msg.jid is " << msg.jid << " and test msgList.state starts with " << msgsList.at(loop).jid;
                returnValue = false;
                break;
            }

            loop++;
        }
    }

    if (theSide == lhs)
    {
        stateChangeMsgLhsList_.clear();
    }
    else
    {
        // TODO
    }

    return returnValue;
}


void ClientRoomMsgTest::collectMsgStateChangedLhs(QString msgId, QString jid, int state)
{
    qDebug() << "collectMsgStateChanged: " << msgId << ", " << jid << ", " << state;
    MsgIdJidState mjs{msgId, jid, state};
    stateChangeMsgLhsList_.push_back(mjs);
}

QTEST_MAIN(ClientRoomMsgTest)
