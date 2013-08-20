/*
 * Copyright (C) 2012-2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.0
import Ubuntu.Components.ListItems 0.1 as ListItem
import Ubuntu.OnlineAccounts 0.1

/*!
*/
Item {
    id: sharemenu

    height: childrenRect.height

    signal selected(string accountId)

    ListView {
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: childrenRect.height
        interactive: false

        model: AccountServiceModel {
            serviceType: "microblogging"
            includeDisabled: true
        }

        delegate: Item {
            width: parent.width
            height: childrenRect.height
            visible: serviceName == "Facebook"

            AccountService {
                id: service
                objectHandle: accountService
            }

            ListItem.Subtitled {
                text: service.provider.displayName
                subText: displayName
                icon: "image://gicon/" + service.provider.iconName
                __iconHeight: units.gu(5)
                __iconWidth: units.gu(5)

                onClicked: {
                    if (service.provider.displayName == "Facebook") {
                        sharemenu.selected(accountId);
                    } else {
                        console.log("Sharing to this service is not supported yet.");
                        sharemenu.selected(null);
                    }
                }
            }
        }
    }
}
