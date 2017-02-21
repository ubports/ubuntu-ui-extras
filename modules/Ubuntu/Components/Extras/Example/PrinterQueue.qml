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
 *             Andrew Hayzen <andrew.hayzen@canonical.com>
 */

import QtQuick 2.4
import QtQuick.Layouts 1.1
import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3 as ListItems
import Ubuntu.Settings.Components 0.1
import Ubuntu.Components.Extras.Printers 0.1

MainView {
    width: units.gu(50)
    height: units.gu(90)

    Component {
        id: queuePage

        Page {
            header: PageHeader {
                title: "Queue: " + printer.name
                flickable: queueView
            }
            visible: false

            property var printer

            ListView {
                id: queueView
                anchors {
                    fill: parent
                }
                delegate: ListItem {
                    height: modelLayout.height + (divider.visible ? divider.height : 0)
                    ListItemLayout {
                        id: modelLayout
                        title.text: displayName
                        subtitle.text: "Job: " + model.id + " State: " + model.state
                                       + " Color: " + model.colorModel + " CreationTime: "
                                       + model.creationTime + " PageRange: "
                                       + model.printRange + " Messages: " + model.messages;
                        subtitle.wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        subtitle.maximumLineCount: 3
                    }
                    onClicked: {
                        console.debug("Cancel:", printer.name, model.id);
                        Printers.cancelJob(printer.name, model.id);
                    }
                }
                model: printer.jobs

                Label {
                    anchors {
                        centerIn: parent
                    }
                    text: "Empty queue"
                    visible: queueView.count === 0
                }
            }
        }
    }

    PageStack {
        id: pageStack

        Page {
            id: printersPage
            header: PageHeader {
                title: "Printers"
                flickable: printerList
            }
            visible: false

            ListView {
                id: printerList
                anchors { fill: parent }
                model: Printers.allPrintersWithPdf
                delegate: ListItem {
                    height: modelLayout.height + (divider.visible ? divider.height : 0)
                    ListItemLayout {
                        id: modelLayout
                        title.text: displayName
                        title.font.bold: model.default
                        subtitle.text: description

                        Icon {
                            id: icon
                            width: height
                            height: units.gu(2.5)
                            name: "printer-symbolic"
                            SlotsLayout.position: SlotsLayout.First
                        }

                        ProgressionSlot {}
                    }
                    onClicked: pageStack.push(queuePage, { printer: model })
                }
            }
        }

        Component.onCompleted: push(printersPage)
    }
}
