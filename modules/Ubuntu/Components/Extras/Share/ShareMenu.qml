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

    Component {
        id: itemDelegate
        Item {
            anchors {
                left: parent.left
                right: parent.right
            }
            AccountService {
                id: service
                objectHandle: accountServiceHandle
            }

            height: childrenRect.height

            ListItem.Subtitled {
                anchors {
                    left: parent.left
                    right: parent.right
                }

                text: service.provider.displayName
                subText: displayName
                iconName: service.provider.iconName
                __iconHeight: units.gu(5)
                __iconWidth: units.gu(5)

                onClicked: {
                    sharemenu.selected(accountId);
                }
            }
        }
    }

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
            provider: "facebook"
        }
        delegate: itemDelegate
    }
}
