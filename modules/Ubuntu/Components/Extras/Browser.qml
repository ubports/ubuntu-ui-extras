/*
 * Copyright 2013 Canonical Ltd.
 *
 * This file is part of webbrowser-app.
 *
 * webbrowser-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * webbrowser-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import QtWebKit 3.0
import Ubuntu.Components 0.1
import Ubuntu.Components.Extras 0.1
import "Browser"

FocusScope {
    id: browser

    property bool chromeless: false
    property string desktopFileHint: ""
    property real qtwebkitdpr
    property bool developerExtrasEnabled: false
    // necessary so that all widgets (including popovers) follow that
    property alias automaticOrientation: orientationHelper.automaticOrientation

    Component.onCompleted: i18n.domain = "ubuntu-ui-extras"

    property alias currentIndex: tabsModel.currentIndex
    property alias currentWebview: tabsModel.currentWebview
    property string title: currentWebview ? currentWebview.title : ""

    focus: true

    OrientationHelper {
        id: orientationHelper

        Item {
            id: webviewContainer

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                bottom: osk.top
            }

        }

        ErrorSheet {
            id: error
            anchors.fill: webviewContainer
            visible: false
            url: currentWebview ? currentWebview.url : ""
            onRefreshClicked: currentWebview.reload()
        }

        ActivityView {
            id: activityView

            anchors.fill: parent
            visible: false
            tabsModel: tabsModel

            onNewTabRequested: {
                browser.newTab("", true)
                visible = false
            }
            onSwitchToTabRequested: {
                browser.switchToTab(index)
                visible = false
            }
            onCloseTabRequested: {
                browser.closeTab(index)
                if (tabsModel.count == 0) {
                    newTabRequested()
                }
            }
        }

        Loader {
            id: panel

            property Item chrome: item ? item.contents[0] : null

            sourceComponent: browser.chromeless ? undefined : panelComponent

            anchors {
                left: parent.left
                right: parent.right
                bottom: (item && item.opened) ? osk.top : parent.bottom
            }

            Component {
                id: panelComponent

                Panel {
                    anchors {
                        left: parent ? parent.left : undefined
                        right: parent ? parent.right : undefined
                        bottom: parent ? parent.bottom : undefined
                    }
                    height: units.gu(8)

                    opened: true

                    Chrome {
                        anchors.fill: parent

                        url: currentWebview ? currentWebview.url : ""

                        loading: currentWebview ? currentWebview.loading || (currentWebview.loadProgress == 0) : false
                        loadProgress: currentWebview ? currentWebview.loadProgress : 0

                        canGoBack: currentWebview ? currentWebview.canGoBack : false
                        onGoBackClicked: currentWebview.goBack()

                        canGoForward: currentWebview ? currentWebview.canGoForward : false
                        onGoForwardClicked: currentWebview.goForward()

                        onUrlValidated: currentWebview.url = url

                        property bool stopped: false
                        onLoadingChanged: {
                            if (loading) {
                                if (panel.item) {
                                    panel.item.opened = true
                                }
                            } else if (stopped) {
                                stopped = false
                            } else if (!addressBar.activeFocus) {
                                if (panel.item) {
                                    panel.item.opened = false
                                }
                                if (currentWebview) {
                                    currentWebview.forceActiveFocus()
                                }
                            }
                        }

                        onRequestReload: currentWebview.reload()
                        onRequestStop: {
                            stopped = true
                            currentWebview.stop()
                        }

                        onToggleTabsClicked: {
                            activityView.visible = !activityView.visible
                            if (activityView.visible) {
                                currentWebview.forceActiveFocus()
                                panel.item.opened = false
                            }
                        }
                    }
                }
            }
        }

        Suggestions {
            visible: panel.chrome && panel.chrome.addressBar.activeFocus && (count > 0)
            anchors {
                bottom: panel.top
                horizontalCenter: parent.horizontalCenter
            }
            width: panel.width - units.gu(5)
            height: Math.min(contentHeight, panel.y - units.gu(2))
            onSelected: {
                currentWebview.url = url
                currentWebview.forceActiveFocus()
            }
        }

        KeyboardRectangle {
            id: osk
        }
    }


    TabsModel {
        id: tabsModel
    }

    Component {
        id: webviewComponent

        UbuntuWebView {
            id: webview

            anchors.fill: parent

            visible: tabsModel.currentWebview === webview

            devicePixelRatio: browser.qtwebkitdpr

            experimental.preferences.developerExtrasEnabled: browser.developerExtrasEnabled

            onLoadingChanged: {
                if (visible) {
                    error.visible = (loadRequest.status === WebView.LoadFailedStatus)
                }
                if (loadRequest.status === WebView.LoadSucceededStatus) {
                    historyModel.add(webview.url, webview.title, webview.icon)
                }
            }
        }
    }

    function newTab(url, setCurrent) {
        var webview = webviewComponent.createObject(webviewContainer, {"url": url})
        var index = tabsModel.add(webview)
        if (setCurrent) {
            tabsModel.currentIndex = index
            if (!browser.chromeless) {
                if (!url) {
                    panel.chrome.addressBar.forceActiveFocus()
                    panel.item.opened = true
                }
            }
        }
    }

    function closeTab(index) {
        var webview = tabsModel.remove(index)
        if (webview) {
            webview.destroy()
        }
    }

    function switchToTab(index) {
        tabsModel.currentIndex = index
        currentWebview.forceActiveFocus()
    }
}
