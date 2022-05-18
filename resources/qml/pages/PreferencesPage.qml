import QtQuick 2.0
import Sailfish.Silica 1.0
import QtQuick.XmlListModel 2.0

Page {
    id: page;
    allowedOrientations: Orientation.All;

    PageHeader {
        id: ph
        title: qsTr("Settings")
    }

    SilicaListView
    {
        id: listView

        anchors {
            top: ph.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        clip: true
        spacing: Theme.paddingMedium;


        header: Column {
            id: column
            width: parent.width

            SectionHeader { text: qsTr("Notifications") }

            TextSwitch {
                id: chatNotificationSwitch
                checked: shmoose.settings.DisplayChatNotifications
                text: qsTr("Display chat notifications")
                onClicked: {
                    shmoose.settings.DisplayChatNotifications = chatNotificationSwitch.checked;
                }
            }
            TextSwitch {
                id: groupchatNotificationSwitch
                checked: shmoose.settings.DisplayGroupchatNotifications
                text: qsTr("Display group chat notifications")
                onClicked: {
                    shmoose.settings.DisplayGroupchatNotifications = groupchatNotificationSwitch.checked;
                }
            }

            SectionHeader { text: qsTr("Privacy") }

            TextSwitch {
                id: readNotificationSwitch
                checked: shmoose.settings.SendReadNotifications
                text: qsTr("Send Read Notifications")
                onClicked: {
                    shmoose.settings.SendReadNotifications = readNotificationSwitch.checked;
                }
            }

            SectionHeader { text: qsTr("Features") }

            TextSwitch {
                id: softwareFeatureOmemoSwitch
                checked: shmoose.settings.EnableSoftwareFeatureOmemo
                text: qsTr("Omemo Message Encryption - Experimental! (Need app restart)")
                onClicked: {
                    shmoose.settings.EnableSoftwareFeatureOmemo = softwareFeatureOmemoSwitch.checked;
                }
            }

            Label
            {
                text: qsTr("Fingerprints of my devices")
                width: parent.width
                visible: dlModel.count > 0
                leftPadding: Theme.horizontalPageMargin
                topPadding: Theme.paddingLarge
                bottomPadding: Theme.paddingMedium
            }
        }

        model: XmlListModel {
            id: dlModel
            xml: shmoose.getFingerprints("")
            query: "/devices/device"
            XmlRole { name: "currentDevice"; query: "boolean(@current)" }
            XmlRole { name: "deviceFp"; query: "fingerprint/string()" }
        }

        delegate: ListItem {

            id: item
            contentHeight: layout.height + thisDev.height

            Label {
                id: layout
                width: listView.width
                text: deviceFp
                leftPadding: Theme.horizontalPageMargin
                wrapMode: TextEdit.WordWrap
                font.family: "Monospace"
                font.pixelSize: Theme.fontSizeSmall
            }
            Label {
                id: thisDev
                width: listView.width
                anchors.top: layout.bottom
                leftPadding: Theme.horizontalPageMargin
                text: qsTr("This is current device")
                visible: currentDevice
                font.pixelSize: Theme.fontSizeTiny
            }

            MouseArea {
                anchors.fill: parent

                onPressAndHold: {
                    item.openMenu();
                }
            }

            menu: ContextMenu {
                MenuItem {
                    text: qsTr("Copy to Clipboard")
                    onClicked: Clipboard.text = deviceFp
                }
            }
        }
    }
}