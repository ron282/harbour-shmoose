import QtQuick 2.0 
import QtQuick.Window 2.0;
import Sailfish.Silica 1.0 
import harbour.shmoose 1.0 
import QtQuick.XmlListModel 2.0

Page {
    id: page
    allowedOrientations: Orientation.All
    property string conversationId
    readonly property int limitCompression : shmoose.settings.LimitCompression
    property int maxUploadSize : shmoose.getMaxUploadSize();

    Timer {
        interval: 2000; running: true; repeat: true
        onTriggered: {
            maxUploadSize = shmoose.getMaxUploadSize();
        }
    }

    PageHeader {
        id: ph
        title: conversationId+qsTr(" settings")
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


        header: Column
        {
            anchors {
                left: parent.left
                right: parent.right
            }

            SectionHeader { text: qsTr("Notifications") }

            ComboBox {
                label: qsTr("Chat notifications")
                width: parent.width
                currentIndex: (
                    shmoose.settings.ForceOnNotifications.indexOf(conversationId) >= 0 ? 1 :
                    shmoose.settings.ForceOffNotifications.indexOf(conversationId) >= 0 ? 2 :
                    0
                )
                menu: ContextMenu {
                    MenuItem {
                        text: qsTr("Default setting")
                        onClicked: {
                            shmoose.settings.removeForceOnNotifications(conversationId);
                            shmoose.settings.removeForceOffNotifications(conversationId);
                        }
                    }
                    MenuItem {
                        text: qsTr("On")
                        onClicked: {
                            shmoose.settings.addForceOnNotifications(conversationId);
                            shmoose.settings.removeForceOffNotifications(conversationId);
                        }
                    }
                    MenuItem {
                        text: qsTr("Off")
                        onClicked: {
                            shmoose.settings.removeForceOnNotifications(conversationId);
                            shmoose.settings.addForceOffNotifications(conversationId);
                        }
                    }
                }
            }


            SectionHeader { text: qsTr("Attachments") }

            TextSwitch {
                id: compressImagesSwitch
                checked: shmoose.settings.CompressImages
                text: qsTr("Limit compression to")
                onClicked: {
                    shmoose.settings.CompressImages = compressImagesSwitch.checked;
                    limitCompressionSizeSlider.enabled = compressImagesSwitch.checked;
                }
            }

            Slider {
                id: limitCompressionSlider
                enabled: shmoose.settings.CompressImages
                width: parent.width
                minimumValue: 100000
                maximumValue: Math.max(maxUploadSize, limitCompression)
                stepSize: 100000
                value: limitCompression
                valueText: value/1000 + qsTr(" KB")

                onValueChanged: {
                    shmoose.settings.LimitCompression = sliderValue;
                }
            }

            TextSwitch {
                id: sendOnlyImagesSwitch
                checked: shmoose.settings.SendOnlyImages
                text: qsTr("Send images only")
                onClicked: {
                    shmoose.settings.SendOnlyImages = sendOnlyImagesSwitch.checked;
                }
            }

            SectionHeader { text: qsTr("OMEMO Encryption") }

            TextSwitch {
                id: sendOmemoMsg
                enabled: shmoose.isOmemoUser(conversationId)
                checked: {
                    if ( shmoose.isOmemoUser(conversationId) === false) {
                        return false;
                    }
                    else if (shmoose.settings.SendPlainText.indexOf(conversationId) >= 0) {
                        return false;
                    }
                    else {
                        return true;
                    }
                }
                text: qsTr("Send encrypted messages")
                onClicked: {
                    if (sendOmemoMsg.checked) {
                        shmoose.settings.removeForcePlainTextSending(conversationId)
                    }
                    else {
                        shmoose.settings.addForcePlainTextSending(conversationId)
                    }
                }
            }

            Label
            {
                text: qsTr("Messages are sent to ") + dlModel.count + qsTr(" device(s) with the following fingerprint(s):")
                visible: dlModel.count > 0
                width: parent.width
                wrapMode: TextEdit.WordWrap
                leftPadding: Theme.horizontalPageMargin
                rightPadding: Theme.horizontalPageMargin
                topPadding: Theme.paddingLarge
                bottomPadding: Theme.paddingMedium
            }
        }

        model: XmlListModel {
            id: dlModel
            xml: shmoose.getFingerprints(conversationId)
            query: "/devices/device"
            XmlRole { name: "deviceFp"; query: "fingerprint/string()" }
        }

        delegate: ListItem {

            id: item
            contentHeight: layout.height

            Label {
                id: layout
                width: listView.width
                text: deviceFp
                wrapMode: TextEdit.WordWrap
                leftPadding: Theme.horizontalPageMargin
                font.family: "Monospace"
                font.pixelSize: Theme.fontSizeSmall
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
