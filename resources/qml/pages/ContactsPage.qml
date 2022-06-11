import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.shmoose 1.0

Page {
    id: page;
    allowedOrientations: Orientation.All;

//    Component {
//        id: sectionHeading
//        Rectangle {
//            width: container.width
//            height: childrenRect.height
//
//            required property string section
//
//            Label {
//                text: parent.section
//                font.pixelSize: Theme.fontSizeLarge
//            }
//        }
//    }

    SilicaListView {
        id: jidlist
        header: Column {
            spacing: Theme.paddingMedium;
            anchors {
                left: parent.left;
                right: parent.right;
            }

            PageHeader {
                title: qsTr("Contacts");
            }
            //            SearchField {
            //                placeholderText: qsTr ("Filter");
            //                anchors {
            //                    left: parent.left;
            //                    right: parent.right;
            //                }
            //            }
        }
        model: shmoose.rosterController.rosterList
        spacing: Theme.paddingLarge

        delegate: ListItem {
            id: item;
            menu: contextMenu
            contentHeight: Theme.itemSizeMedium;
            onClicked: {
                shmoose.setCurrentChatPartner(jid)
                pageStack.push (pageMessaging, { "conversationId" : jid });
            }

            Image {
                id: img;
                width: height;
                source: imagePath != "" ? imagePath : getImage(jid)
                anchors {
                    top: parent.top;
                    left: parent.left;
                    bottom: parent.bottom;
                }

                Rectangle {
                    z: -1;
                    color: (model.index % 2 ? "black" : "white");
                    opacity: 0.15;
                    anchors.fill: parent;
                }
            }
            Column {
                anchors {
                    left: img.right;
                    right: parent.right
                    margins: Theme.paddingMedium;
                    verticalCenter: parent.verticalCenter;
                }

                Label {
                    id: nameId;
                    text: name
                    wrapMode: Text.NoWrap
                    maximumLineCount: 1
                    color: (item.highlighted ? Theme.highlightColor : Theme.primaryColor);
                    font.pixelSize: Theme.fontSizeMedium;
                }
                Row {
                    width: parent.width

                    Rectangle {
                        id: presence
                        anchors {
                            bottom: img.bottom
                            right: img.right
                            bottomMargin: Theme.paddingSmall
                            rightMargin: Theme.paddingSmall
                        }
                        width: Math.max(lbl.implicitWidth+radius, Theme.iconSizeSmall)
                        height: Theme.iconSizeExtraSmall
                        radius: height*0.5
                        color: availability === RosterItem.AVAILABILITY_ONLINE ? "green" : "gray"
                        Label {
                            id: lbl
                            font.bold: true
                            text: availability !== RosterItem.AVAILABILITY_ONLINE && availability !== RosterItem.AVAILABILITY_OFFLINE ? "?" : ""
                            font.pixelSize: Theme.fontSizeTiny
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                    Image {
                        id: subscriptionImage;
                        visible: ! shmoose.rosterController.isGroup(jid)
                        source: getSubscriptionImage(subscription);
                    }
                    Image {
                        visible: false
                        id: availabilityImage;
                        source: getAvailabilityImage(availability)
                    }
                    Label {
                        id: jidId;
                        text: jid;
                        width: parent.width - subscriptionImage.width - presence.width - 2*Theme.paddingMedium
                        truncationMode: TruncationMode.Fade
                        color: Theme.secondaryColor;
                        font.pixelSize: Theme.fontSizeTiny;
                    }
                }
                Label {
                    id: statusId;
                    text: status;
                    color: Theme.secondaryColor;
                    font.pixelSize: Theme.fontSizeTiny;
                }
                Component {
                    id: contextMenu
                    ContextMenu {
                        MenuItem {
                            text:  qsTr("Remove");
                            onClicked: {
                                remorseAction(qsTr("Remove contact"),
                                function() {
                                    if (shmoose.rosterController.isGroup(jid)) {
                                        shmoose.removeRoom(jid)
                                    }
                                    else {
                                        shmoose.rosterController.removeContact(jid)
                                    }
                                })  
                            }
                        }
                    }
                }
            }
        }

        anchors.fill: parent;

        PullDownMenu {
            enabled: true
            visible: true

            //            MenuItem {
            //                text: qsTr ("Create Room TBD");
            //                onClicked: {
            //                    console.log("create room")
            //                }
            //            }

            MenuItem {
                text: qsTr("Join room by address");
                onClicked: {
                    pageStack.push(dialogJoinRoom)
                }
            }

            MenuItem {
                text: qsTr("Add contact");
                onClicked: {
                    pageStack.push(dialogCreateContact)
                }
            }

        }

//        section.property: "name"
//        section.criteria: ViewSection.FirstCharacter
//        section.delegate: sectionHeading
    }

    function getImage(jid) {
        if (shmoose.rosterController.isGroup(jid)) {
            return "image://theme/icon-l-image";
        } else {
            return "image://theme/icon-l-people"
        }
    }

    function getSubscriptionImage(subs) {
        if (subs === RosterItem.SUBSCRIPTION_NONE) {
            return "image://theme/icon-cover-cancel"
        } else if (subs === RosterItem.SUBSCRIPTION_TO) {
            return "image://theme/icon-cover-next"
        } else if (subs === RosterItem.SUBSCRIPTION_FROM) {
            return "image://theme/icon-cover-previous"
        } else {
            return "image://theme/icon-cover-transfers"
        }
    }

    function getAvailabilityImage(avail) {
        if (avail === RosterItem.AVAILABILITY_ONLINE) {
            return "image://theme/icon-s-chat"
        } else if (avail === RosterItem.AVAILABILITY_OFFLINE) {
            return "image://theme/icon-s-high-importance"
        } else {
            return "image://theme/icon-s-timer"
        }
    }

}


