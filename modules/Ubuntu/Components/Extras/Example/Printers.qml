/*
 * Copyright 2017 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by Jonas G. Drange <jonas.drange@canonical.com>
 */

import QtQuick 2.4
import QtQuick.Layouts 1.1
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3
import Ubuntu.Components.ListItems 1.3 as ListItems
import Ubuntu.Components.Extras.Printers 0.1

MainView {
    width: units.gu(50)
    height: units.gu(90)

    Component {
        id: printerPage

        Page {
            visible: false
            property var printer
            header: PageHeader {
                id: printerPageHeader
                title: printer.name
                flickable: printerFlickable
            }

            Component {
                id: printerPageNotYetLoaded

                Item {
                    anchors.fill: parent
                    ActivityIndicator {
                        anchors.centerIn: parent
                        running: true
                    }
                }
            }

            Flickable {
                id: printerFlickable
                anchors.fill: parent
                contentHeight: contentItem.childrenRect.height
                Loader {
                    id: printerPageBitsLoader
                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    sourceComponent: printer.isLoaded ? printerPageLoaded : printerPageNotYetLoaded
                }
            }

            Component {
                id: printerPageLoaded

                Column {
                    height: childrenRect.height + anchors.topMargin
                    anchors {
                        top: parent.top
                        topMargin: units.gu(2)
                        left: parent.left
                        right: parent.right
                    }

                    ListItems.Standard {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        text: "Enabled"

                        control: Switch {
                            checked: printer.printerEnabled
                            onCheckedChanged: printer.printerEnabled = checked
                        }
                    }

                    ListItems.SingleValue {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        text: "Status"

                        value: {
                            var state;
                            if (printer.state == PrinterEnum.IdleState) {
                                state = i18n.tr("Idle");
                            } else if (printer.state == PrinterEnum.AbortedState) {
                                state = i18n.tr("Aborted");
                            } else if (printer.state == PrinterEnum.ActiveState) {
                                state = i18n.tr("Active");
                            } else if (printer.state == PrinterEnum.ErrorState) {
                                state = i18n.tr("Stopped");
                            }
                            return "%1 — %2"
                                .arg(state)
                                .arg(printer.lastMessage ?
                                     printer.lastMessage : i18n.tr("No messages"));
                        }
                    }

                    ListItems.Standard {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        text: "Accepting jobs"

                        control: Switch {
                            checked: printer.acceptJobs
                            onCheckedChanged: printer.acceptJobs = checked
                        }
                    }

                    ListItems.Standard {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        text: "Shared"

                        control: Switch {
                            checked: printer.shared
                            onCheckedChanged: printer.shared = checked
                        }
                    }

                    ListItems.Standard {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        text: "Jobs"
                        progression: true
                        onClicked: pageStack.push(jobPage, { printer: printer })
                    }

                    ListItems.Standard {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        text: "Description"

                        control: TextField {
                           anchors {
                                margins: units.gu(1)
                                left: parent.left
                                right: parent.right

                            }
                            text: printer.description
                            onTextChanged: printer.description = text
                        }
                    }

                    ListItems.Standard {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        text: "Copies"

                        control: TextField {
                            id: copiesField
                            inputMethodHints: Qt.ImhDigitsOnly
                            text: printer.copies
                            validator: IntValidator {
                                bottom: 1
                                top: 999
                            }
                            width: units.gu(10)
                            onTextChanged: printer.copies = text
                        }

                    }


                    ListItems.ValueSelector {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        enabled: values.length > 1
                        text: "Duplex"
                        values: printer.supportedDuplexModes
                        onSelectedIndexChanged: printer.duplexMode = selectedIndex
                        Component.onCompleted: {
                            if (enabled) {
                                selectedIndex = printer.duplexMode
                            }
                        }
                    }

                    ListItems.ValueSelector {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        text: "Page size"
                        values: printer.supportedPageSizes
                        onSelectedIndexChanged: printer.pageSize = selectedIndex
                        Component.onCompleted: selectedIndex = printer.supportedPageSizes.indexOf(printer.pageSize)
                    }

                    ListItems.ValueSelector {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        visible: printer.supportedColorModels.length
                        text: "Color model"
                        values: printer.supportedColorModels
                        enabled: values.length > 1
                        onSelectedIndexChanged: printer.colorModel = selectedIndex
                        Component.onCompleted: {
                            if (enabled)
                                selectedIndex = printer.colorModel
                        }
                    }

                    ListItems.ValueSelector {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        visible: printer.supportedPrintQualities.length
                        text: "Quality"
                        values: printer.supportedPrintQualities
                        enabled: values.length > 1
                        onSelectedIndexChanged: printer.printQuality = selectedIndex
                        Component.onCompleted: {
                            if (enabled)
                                selectedIndex = printer.printQuality
                        }
                    }

                    ListItems.SingleControl {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        control: Button {
                            text: "Print test page"
                            onClicked: Printers.printTestPage(printer.name)
                        }
                    }

                    ListItems.SingleValue {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        text: "Device URI"
                        value: printer.deviceUri
                    }
                }
            }
        }
    }

    Component {
        id: jobDelegate

        ListItem {
            height: modelLayout.height + (divider.visible ? divider.height : 0)
            trailingActions: ListItemActions {
                actions: [
                    Action {
                        iconName: model.held ? "media-playback-start" : "media-playback-pause"
                        text: model.held ? "Release" : "Hold"

                        onTriggered: {
                            if (model.held) {
                                Printers.releaseJob(model.printerName, model.id);
                            } else {
                                Printers.holdJob(model.printerName, model.id);
                            }
                        }
                    },
                    Action {
                        iconName: "cancel"
                        text: "Cancel"

                        onTriggered: Printers.cancelJob(model.printerName, model.id);
                    }
                ]
            }

            ListItemLayout {
                id: modelLayout
                title.text: displayName
                subtitle.text: "Printing " + model.impressionsCompleted + " pages" + "\n" + model.printerName
                subtitle.wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                subtitle.maximumLineCount: 3
            }
        }
    }

    Component {
        id: jobPage
        Page {
            property var printer
            header: PageHeader {
                id: jobPageHeader
                title: "%1 (%2 jobs)".arg(printer.name).arg(jobList.count)
                flickable: jobList
            }

            ListView {
                id: jobList
                anchors.fill: parent
                model: printer.jobs
                delegate: jobDelegate
            }
        }
    }

    Component {
        id: allJobsPage
        Page {
            header: PageHeader {
                id: allJobsHeader
                title: "Printer jobs"
                flickable: jobsList
            }

            ListView {
                id: jobsList
                anchors.fill: parent
                model: Printers.printJobs
                delegate: jobDelegate
            }
        }
    }


    PageStack {
        id: pageStack

        Component.onCompleted: push(printersPage)

        Page {
            id: printersPage
            header: PageHeader {
                title: "Printers"
                flickable: printerList
                trailingActionBar {
                    actions: [
                        Action {
                            iconName: "add"
                            text: "Add printer"
                            onTriggered: pageStack.push(addPrinterPageComponent)
                        },
                        Action {
                            iconName: "document-print"
                            text: "Printer jobs"
                            onTriggered: pageStack.push(allJobsPage)
                        }
                    ]
                }
            }
            visible: false

            ListView {
                id: printerList
                anchors { fill: parent }
                model: Printers.allPrintersWithPdf
                delegate: ListItem {
                    height: modelLayout.height + (divider.visible ? divider.height : 0)
                    trailingActions: ListItemActions {
                        actions: [
                            Action {
                                iconName: "delete"
                                onTriggered: {
                                    if (!Printers.removePrinter(model.name)) {
                                        console.error('failed to remove printer', Printers.lastMessage);
                                    }
                                }
                            },
                            Action {
                                iconName: model.default ? "starred" : "non-starred"
                                enabled: !model.default
                                onTriggered: Printers.defaultPrinterName = model.name
                            }

                        ]
                    }
                    ListItemLayout {
                        id: modelLayout
                        title.text: displayName
                        title.font.bold: model.default

                        Icon {
                            id: icon
                            width: height
                            height: units.gu(2.5)
                            name: "printer-symbolic"
                            SlotsLayout.position: SlotsLayout.First
                        }

                        ProgressionSlot {}
                    }
                    onClicked: {
                        Printers.loadPrinter(model.name);
                        pageStack.push(printerPage, { printer: model });
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: printerList.count === 0
                text: "No printers found"
            }
        }
    }

    Component {
        id: addPrinterPageComponent
        Page {
            id: addPrinterPage
            states: [
                State {
                    name: "success"
                    PropertyChanges {
                        target: okAction
                        enabled: false
                    }
                    PropertyChanges {
                        target: closeAction
                        enabled: false
                    }
                    PropertyChanges {
                        target: addPrinterCol
                        enabled: false
                    }
                    StateChangeScript {
                        script: okTimer.start()
                    }
                },
                State {
                    name: "failure"
                    PropertyChanges {
                        target: errorMessageContainer
                        visible: true
                    }
                }
            ]
            header: PageHeader {
                title: "Add printer"
                flickable: addPrinterFlickable
                leadingActionBar.actions: [
                    Action {
                        id: closeAction
                        iconName: "close"
                        text: "Abort"
                        onTriggered: pageStack.pop()
                    }
                ]
                trailingActionBar {
                    actions: [
                        Action {
                            id: okAction
                            iconName: "ok"
                            text: "Complete"
                            onTriggered: {
                                var ret;
                                if (driverSelector.selectedIndex == 0) {
                                    ret = Printers.addPrinter(
                                        printerName.text,
                                        driversView.selectedDriver,
                                        printerUri.text,
                                        printerDescription.text,
                                        printerLocation.text
                                    );
                                } else {
                                    ret = Printers.addPrinterWithPpdFile(
                                        printerName.text,
                                        printerPpd.text,
                                        printerUri.text,
                                        printerDescription.text,
                                        printerLocation.text
                                    );
                                }
                                if (ret) {
                                    addPrinterPage.state = "success"
                                } else {
                                    errorMessage.text = Printers.lastMessage;
                                    addPrinterPage.state = "failure"
                                }
                            }
                        }
                    ]
                }
            }

            Component.onCompleted: Printers.prepareToAddPrinter()

            Timer {
                id: okTimer
                interval: 2000
                onTriggered: pageStack.pop();
            }

            Flickable {
                id: addPrinterFlickable
                anchors.fill: parent
                contentHeight: contentItem.childrenRect.height

                Column {
                    id: addPrinterCol
                    property bool enabled: true
                    anchors { left: parent.left; right: parent.right }

                    Item {
                        id: errorMessageContainer
                        visible: false
                        anchors {
                            left: parent.left
                            right: parent.right
                            margins: units.gu(2)
                        }
                        height: units.gu(6)
                        Label {
                            id: errorMessage
                            anchors {
                                top: parent.top
                                topMargin: units.gu(2)
                                horizontalCenter: parent.horizontalCenter
                            }
                        }

                    }

                    ListItems.Standard {
                        text: "Device URI"
                        control: TextField {
                            id: printerUri
                            placeholderText: "ipp://server.local/my-queue"
                        }
                        enabled: parent.enabled
                    }

                    ListItems.ValueSelector {
                        id: driverSelector
                        anchors { left: parent.left; right: parent.right }
                        text: "Choose driver"
                        values: [
                            "Select printer from database",
                            "Provide PPD file"
                        ]
                        enabled: parent.enabled
                    }

                    ListItems.Standard {
                        anchors { left: parent.left; right: parent.right }
                        text: "Filter drivers"
                        control: TextField {
                            id: driverFilter
                            onTextChanged: Printers.driverFilter = text
                        }
                        visible: driverSelector.selectedIndex == 0
                        enabled: parent.enabled
                    }

                    ListView {
                        id: driversView
                        property string selectedDriver
                        property bool loading: true
                        visible: driverSelector.selectedIndex == 0
                        model: Printers.drivers
                        anchors { left: parent.left; right: parent.right }
                        height: units.gu(30)
                        clip: true
                        enabled: parent.enabled
                        highlightFollowsCurrentItem: false
                        highlight: Rectangle {
                            z: 0
                            y: driversView.currentItem.y
                            width: driversView.currentItem.width
                            height: driversView.currentItem.height
                            color: theme.palette.selected.background
                        }
                        delegate: ListItem {
                            height: driverLayout.height + (divider.visible ? divider.height : 0)
                            ListItemLayout {
                                id: driverLayout
                                title.text: displayName
                                subtitle.text: name
                                summary.text: deviceId
                            }
                            onClicked: {
                                driversView.selectedDriver = name
                                driversView.currentIndex = index
                            }
                        }

                        ActivityIndicator {
                            anchors.centerIn: parent
                            running: parent.loading
                        }

                        Connections {
                            target: driversView
                            onCountChanged: {
                                target = null;
                                driversView.loading = false;
                            }
                        }
                    }

                    ListItems.Standard {
                        text: "PPD File"
                        visible: driverSelector.selectedIndex == 1
                        control: TextField {
                            id: printerPpd
                            placeholderText: "/usr/share/cups/foo.ppd"
                        }
                        enabled: parent.enabled
                    }

                    ListItems.Standard {
                        anchors { left: parent.left; right: parent.right }
                        text: "Printer name"
                        control: TextField {
                            id: printerName
                            placeholderText: "laserjet"
                        }
                        enabled: parent.enabled
                    }

                    ListItems.Standard {
                        anchors { left: parent.left; right: parent.right }
                        text: "Description (optional)"
                        control: TextField {
                            id: printerDescription
                            placeholderText: "HP Laserjet with Duplexer"
                        }
                        enabled: parent.enabled
                    }

                    ListItems.Standard {
                        anchors { left: parent.left; right: parent.right }
                        text: "Location (optional)"
                        control: TextField {
                            id: printerLocation
                            placeholderText: "Lab 1"
                        }
                        enabled: parent.enabled
                    }
                }

                Label {
                    id: remotePrintersLabel
                    anchors {
                        left: parent.left
                        right: parent.right
                        margins: units.gu(2)
                        top: addPrinterCol.bottom
                    }
                    text: "Other printers"

                    ActivityIndicator {
                        id: remotePrintersSearchIndicator
                        anchors {
                            right: parent.right
                            verticalCenter: parent.verticalCenter
                        }
                        property var target
                        Component.onCompleted: target = Printers.devices
                        running: target.searching
                    }
                }

                ListView {
                    id: remotePrintersList
                    anchors {
                        left: parent.left
                        right: parent.right
                        top: remotePrintersLabel.bottom
                        topMargin: units.gu(2)
                    }
                    height: contentItem.childrenRect.height
                    model: Printers.devices
                    delegate: ListItem {
                        height: modelLayout.height + (divider.visible ? divider.height : 0)
                        ListItemLayout {
                            id: modelLayout
                            title.text: displayName
                            subtitle.text: {
                                if (type == PrinterEnum.LPDType) return "LPD";
                                if (type == PrinterEnum.IppSType) return "IppS";
                                if (type == PrinterEnum.Ipp14Type) return "Ipp14";
                                if (type == PrinterEnum.HttpType) return "Http";
                                if (type == PrinterEnum.BehType) return "Beh";
                                if (type == PrinterEnum.SocketType) return "Socket";
                                if (type == PrinterEnum.HttpsType) return "Https";
                                if (type == PrinterEnum.IppType) return "Ipp";
                                if (type == PrinterEnum.HPType) return "HP";
                                if (type == PrinterEnum.USBType) return "USB";
                                if (type == PrinterEnum.HPFaxType) return "HPFax";
                                if (type == PrinterEnum.DNSSDType) return "DNSSD";
                                else return "Unknown protocol";
                            }

                            Icon {
                                id: icon
                                width: height
                                height: units.gu(2.5)
                                name: "network-printer-symbolic"
                                SlotsLayout.position: SlotsLayout.First
                            }

                            Button {
                                text: "Select printer"
                                onClicked: {
                                    var suggestedPrinterName = (" " + displayName).slice(1);
                                    suggestedPrinterName = suggestedPrinterName.replace(/\ /g, "\-");
                                    printerUri.text = uri;
                                    printerName.text = suggestedPrinterName;
                                    printerDescription.text = info;
                                    printerLocation.text = location;
                                }
                            }
                        }
                    }
                }

                Label {
                    anchors {
                        left: parent.left
                        right: parent.right
                        top: remotePrintersLabel.bottom
                        topMargin: units.gu(2)
                    }
                    text: "No other printers found"
                    visible: !remotePrintersSearchIndicator.running && remotePrintersList.count == 0
                }
            }
        }
    }
}
