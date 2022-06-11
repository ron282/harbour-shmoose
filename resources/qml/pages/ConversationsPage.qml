import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.shmoose 1.0

Page {
    id: page;
    allowedOrientations: Orientation.All;

    SilicaListView {
        id: view;
        header: Column {
            spacing: Theme.paddingMedium;
            anchors {
                left: parent.left;
                right: parent.right;
            }

            PageHeader {
                title: qsTr("Conversations");
            }
        }
        ViewPlaceholder {
            enabled: view.count == 0
            text: qsTr("Empty")
            hintText: qsTr("Select a contact to start a conversation")
        }
        model: shmoose.persistence.sessionController
        spacing: Theme.paddingLarge
        delegate: ListItem {
            id: item;
            contentHeight: Theme.itemSizeMedium;

            onClicked: {
                console.log("set current char partner: " + jid);
                pageStack.push (pageMessaging, { "conversationId" : jid });
                shmoose.setCurrentChatPartner(jid);
            }

            Image {
                id: img;
                width: height;
                source: getImage(jid)
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
            Rectangle {
                width: Math.max(lblUnread.implicitWidth+radius, height)
                height: lblUnread.implicitHeight
                color: Theme.highlightBackgroundColor
                radius: width*0.5
                anchors {
                    top: img.top
                    right: img.right
                    topMargin: Theme.paddingSmall
                    rightMargin: Theme.paddingSmall
                }
                visible: (unreadmessages > 0) ? true : false
                Label {
                    id: lblUnread
                    font.bold: true
                    text: unreadmessages
                    font.pixelSize: Theme.fontSizeTiny
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            Column {
                anchors {
                    left: img.right;
                    margins: Theme.paddingMedium;
                    verticalCenter: parent.verticalCenter;
                    right: parent.right
                }

                Row {
                    Label {
                        id: nameId;
                        wrapMode: Text.NoWrap
                        maximumLineCount: 1
                        text: shmoose.rosterController.getNameForJid(jid)
                        color: (item.highlighted ? Theme.highlightColor : Theme.primaryColor);
                        font.pixelSize: Theme.fontSizeMedium;
                    }
                }
                Row {
                    width: parent.width
                    Label {
                        id: me
                        visible: lastmsgdir == 0
                        text: qsTr("Me: ")
                        color: Theme.secondaryColor
                        font.pixelSize: Theme.fontSizeSmall
                    }
                    Label {
                        id: statusId
                        text: lastmessage
                        visible: lastmsgtype == "txt"
                        width: me.visible ? parent.width-me.width : parent.width
                        truncationMode: TruncationMode.Fade
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                    }
                    Icon {
                        visible: lastmsgtype !== "txt"
                        width: Theme.iconSizeSmallPlus
                        height: Theme.iconSizeSmallPlus
                        anchors.verticalCenter: me.verticalCenter
                        source: getFileSmallIcon(lastmsgtype)
                    }
                }
            }

            menu: ContextMenu {
                MenuItem {
                    text: qsTr("Delete")
                    onClicked: {
                        remorseAction(qsTr("Delete conversation"),
                                      function() {
                                            shmoose.persistence.removeConversation(jid);
                                        })
                    }
                }
            }
        }

        anchors.fill: parent;

    }

    function getImage(jid) {
        var imagePath = shmoose.rosterController.getAvatarImagePathForJid(jid);

        if (imagePath.length > 0) {
            return imagePath;
        } else if (shmoose.rosterController.isGroup(jid)) {
            return "image://theme/icon-l-image";
        } else {
            return "image://theme/icon-l-people"
        }
    }
    function getFileSmallIcon(type) {
        if(startsWith(type, "image")) return "image://theme/icon-m-file-image";
        if(startsWith(type, "video")) return "image://theme/icon-m-file-video";
        if(startsWith(type, "audio")) return "image://theme/icon-m-file-audio";
        return "image://theme/icon-m-file-document-light";
    }
    function startsWith(s,start) {
        return (s.substring(0, start.length) == start);
    }
}


