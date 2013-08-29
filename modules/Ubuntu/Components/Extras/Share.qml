/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import QtQuick.Window 2.0
import Ubuntu.Components 0.1
import Friends 0.2
import Ubuntu.OnlineAccounts 0.1
import Ubuntu.Components.Extras 0.1

Rectangle {
    id: shareComponent
    objectName: "shareComponent"
    color: Theme.palette.normal.background

    property string fileToShare
    property string userAccountId

    signal uploadCompleted(bool success)
    signal canceled()

    FriendsDispatcher {
        id: friends
        onUploadComplete: {
             activitySpinner.visible = false;
             if (success) {
                 notifyOSD.notify(i18n.dtr("ubuntu-ui-extras","Your photo has been uploaded"), null, "facebook");
             } else {
                 notifyOSD.notify(i18n.dtr("ubuntu-ui-extras", "There was a problem uploading your photo"), null, "facebook");
             }
             shareComponent.uploadCompleted(success);
         }
    }

    AccountServiceModel {
        id: accounts
        serviceType: "microblogging"
    }

    ImageResizer {
        id: resizer
        file: fileToShare.replace("file://", "")
        quality: 80
        largestSide: 2048
    }

    Notify {
        id: notifyOSD
        appName: "share-app"
    }

    Item {
        id: serviceHeader
        y: 0
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: units.gu(1)
        anchors.leftMargin: units.gu(1)
        anchors.rightMargin: units.gu(1)
        height: accountsList.height + units.gu(3)

        ListView {
            id: accountsList
            width: parent.width
            anchors.fill: parent
            model: accounts
            delegate: Rectangle {
                width: parent.width
                height: visible ? childrenRect.height : 0
                visible: (accountId == userAccountId)
                AccountService {
                    id: accts
                    objectHandle: accountService
                }

                UbuntuShape {
                    id: logo
                    anchors.left: parent.left
                    anchors.top: parent.top
                    width: units.gu(5)
                    height: units.gu(5)

                    image: Image {
                        fillMode: Image.PreserveAspectFit
                        source: "image://gicon/" + accts.provider.iconName
                        sourceSize.height: logo.height
                        sourceSize.width: logo.width
                    }
                }

                Label {
                    id: serviceTitle
                    anchors.top: parent.top
                    anchors.left: logo.right
                    anchors.leftMargin: units.gu(1)
                    anchors.rightMargin: units.gu(1)

                    text: accts.provider.displayName
                    fontSize: "large"
                    font.weight: Font.DemiBold
                }

                Label {
                    anchors.top: serviceTitle.bottom
                    anchors.left: logo.right
                    anchors.right: parent.right
                    anchors.leftMargin: units.gu(1)
                    anchors.rightMargin: units.gu(1)

                    elide: Text.ElideRight
                    text: displayName
                    fontSize: "medium"
                }
            }
        }
    }

    Rectangle {
        id: topLine
        color: "#cccccc"
        height: units.dp(1)
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: serviceHeader.bottom
    }

    UbuntuShape {
        id: messageArea
        objectName: "messageArea"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: topLine.bottom
        anchors.topMargin: units.gu(1)
        anchors.leftMargin: units.gu(1)
        anchors.rightMargin: units.gu(1)

        height: units.gu(20)
        color: "#f2f2f2"

        TextEdit {
            id: message
            color: "#333333"
            anchors.top: parent.top
            anchors.left: snapshot.right
            anchors.bottom: parent.bottom
            anchors.margins: units.gu(1)
            wrapMode: Text.Wrap
            width: parent.width - snapshot.width -
                   snapshot.anchors.margins * 2 -
                   message.anchors.leftMargin - message.anchors.rightMargin

            clip: true
            font.pixelSize: FontUtils.sizeToPixels("medium")
            font.weight: Font.Light
            focus: true
        }

        ActivityIndicator {
            id: activitySpinner
            anchors.centerIn: message
            visible: false
            running: visible
        }

        UbuntuShape {
            id: snapshot
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: units.gu(1)
            width: units.gu(10)
            height: units.gu(10)

            image: Image {
                source: fileToShare
                sourceSize.height: snapshot.height
                sourceSize.width: snapshot.width
                fillMode: Image.PreserveAspectCrop
            }
        }
    }

    Item {
        id: actionsBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: messageArea.bottom
        anchors.topMargin: units.gu(2)
        anchors.leftMargin: units.gu(1)
        anchors.rightMargin: units.gu(1)
        height: childrenRect.height

        Button {
            objectName: "cancelButton"
            anchors.left: parent.left
            text: i18n.dtr("ubuntu-ui-extras", "Cancel")
            color: "#cccccc"
            width: units.gu(10)
            height: units.gu(4)
            onClicked: shareComponent.canceled()
        }

        Button {
            objectName: "postButton"
            anchors.right: parent.right
            anchors.top: parent.top
            text: i18n.dtr("ubuntu-ui-extras", "Post")
            color: "#dd4814"
            width: units.gu(10)
            height: units.gu(4)
            enabled: !activitySpinner.visible
            onClicked: {
                activitySpinner.visible = true;
                friends.uploadForAccountAsync(userAccountId,
                                              "file://" + resizer.resizedFile,
                                              message.text)
            }
        }
    }

    UbuntuShape {
        id: useLocation
        anchors.left: parent.left
        anchors.top: actionsBar.bottom
        anchors.leftMargin: units.gu(1)
        anchors.topMargin: units.gu(1)
        color: selected ? "#cccccc" : "transparent"
        property bool selected: false
        width: units.gu(4.5)
        height: units.gu(4)

        AbstractButton {
            anchors.fill: parent
            onClicked: parent.selected = !parent.selected

            Image {
                source: "assets/icon_location.png"
                anchors.top: parent.top
                anchors.topMargin: units.gu(0.5)
                anchors.bottom: parent.bottom
                anchors.bottomMargin: units.gu(0.5)
                anchors.horizontalCenter: parent.horizontalCenter
                fillMode: Image.PreserveAspectFit
            }
        }
    }

    Label {
        anchors.left: useLocation.right
        anchors.baseline: useLocation.top
        anchors.baselineOffset: units.gu(3)
        anchors.leftMargin: units.gu(1)
        text: i18n.dtr("ubuntu-ui-extras", "Include location")
        fontSize: "small"
    }

    states: [
        State {
            name: "landscape-with-keyborad"
            PropertyChanges {
                target: serviceHeader
                y: - serviceHeader.height
            }
            PropertyChanges {
                target: messageArea
                height: units.gu(12)
            }
        }
    ]

    state: ((Screen.orientation === Qt.LandscapeOrientation) ||
            (Screen.orientation === Qt.InvertedLandscapeOrientation)) &&
           Qt.inputMethod.visible ? "landscape-with-keyborad" : ""
}
