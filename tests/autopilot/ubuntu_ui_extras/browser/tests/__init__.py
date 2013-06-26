# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2013 Canonical
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.

"""browser autopilot tests."""

import os, glob
import os.path
import shutil
import tempfile
from distutils.sysconfig import get_python_lib

from testtools.matchers import Contains, Equals

from autopilot.input import Mouse, Touch, Pointer
from autopilot.matchers import Eventually
from autopilot.platform import model
from autopilot.testcase import AutopilotTestCase

import http_server

from ubuntu_ui_extras.browser.emulators.main_window import MainWindow


HTTP_SERVER_PORT = 8129
TYPING_DELAY = 0.01


class BrowserTestCaseBase(AutopilotTestCase):

    """
    A common test case class that provides several useful methods
    for the browser component tests.
    """

    if model() == 'Desktop':
        scenarios = [('with mouse', dict(input_device_class=Mouse)), ]
    else:
        scenarios = [('with touch', dict(input_device_class=Touch)), ]

    ARGS = []
    _temp_pages = []
    local_location = "ubuntu_ui_extras/emulators/runner.qml"

    def setUp(self):
        self.pointing_device = Pointer(self.input_device_class.create())
        super(BrowserTestCaseBase, self).setUp()

        if os.path.exists(self.local_location):
            self.launch_test_local()
        else:
            self.launch_test_installed()
        # This is needed to wait for the application to start.
        # In the testfarm, the application may take some time to show up.
        self.assertThat(self.main_window.get_qml_view().visible,
                        Eventually(Equals(True)))

    def tearDown(self):
        super(BrowserTestCaseBase, self).tearDown()
        for page in self._temp_pages:
            try:
                os.remove(page)
            except:
                pass
        self._temp_pages = []

    """Workaround to find the qmlscene binary via shell globbing.
       This is needed since we can't rely on qt5-default being installed on
       devices to make qmlscene available in the path"""
    def qmlscene(self):
        return glob.glob("/usr/lib/*/qt5/bin/qmlscene")[0]

    def launch_test_local(self):
        self.app = self.launch_test_application(self.qmlscene(), "-I", "../../modules", self.local_location,
                                                *self.ARGS)

    def launch_test_installed(self):
        runner = os.path.join(get_python_lib(), "ubuntu_ui_extras/browser/emulators/runner.qml")

        if model() == 'Desktop':
            self.app = self.launch_test_application(self.qmlscene(), runner, *self.ARGS)
        else:
            self.app = self.launch_test_application(self.qmlscene(), runner,
                self.d_f,
                *self.ARGS,
                app_type='qt')

    @property
    def main_window(self):
        return MainWindow(self.app)

    def make_raw_html_page(self, html):
        fd, path = tempfile.mkstemp(suffix=".html", text=True)
        os.write(fd, html)
        os.close(fd)
        self._temp_pages.append(path)
        return "file://" + path

    def make_html_page(self, title, body):
        """
        Write a web page using title and body onto a temporary file,
        and return the corresponding local "file://…" URL. The file
        is automatically deleted after running the calling test method.
        """
        html = "<html><title>%s</title><body>%s</body></html>" % (title, body)
        return self.make_raw_html_page(html)

    def reveal_chrome(self):
        panel = self.main_window.get_panel()
        
        if panel.state != "spread":
            x, y, w, h = panel.globalRect
            tx = x + (w / 2)
            ty = y + (h - 10)
            self.pointing_device.drag(tx, ty, tx, ty - h)
        
        self.assertThat(panel.state, Eventually(Equals("spread")))

    def hide_chrome(self):
        panel = self.main_window.get_panel()

        x, y, w, h = panel.globalRect
        tx = x + (w / 2)
        ty = y + (h / 10)

        self.pointing_device.drag(tx, ty, tx, ty + h - 10)
        self.assertThat(panel.state, Eventually(Equals("")))

    def assert_chrome_eventually_hidden(self):
        panel = self.main_window.get_panel()
        self.assertThat(panel.state, Eventually(Equals("")))


    def ensure_chrome_is_hidden(self):
        panel = self.main_window.get_panel()

        x, y, w, h = panel.globalRect
        tx = x + (w / 2)
        ty = y + (h / 10)

        self.pointing_device.drag(tx, ty, tx, ty + h - 10)
        self.assertThat(panel.state, Eventually(Equals("")))

    def focus_address_bar(self):
        address_bar = self.main_window.get_address_bar()
        self.pointing_device.move_to_object(address_bar)
        self.pointing_device.click()
        self.assertThat(address_bar.activeFocus, Eventually(Equals(True)))

    def clear_address_bar(self):
        self.focus_address_bar()
        clear_button = self.main_window.get_address_bar_clear_button()
        self.pointing_device.move_to_object(clear_button)
        self.pointing_device.click()
        text_field = self.main_window.get_address_bar_text_field()
        self.assertThat(text_field.text, Eventually(Equals("")))

    def assert_chrome_eventually_shown(self):
        panel = self.main_window.get_panel()
        self.assertThat(panel.state, Eventually(Equals("spread")))

    def type_in_address_bar(self, text):
        address_bar = self.main_window.get_address_bar()
        self.assertThat(address_bar.activeFocus, Eventually(Equals(True)))
        self.keyboard.type(text, delay=TYPING_DELAY)
        text_field = self.main_window.get_address_bar_text_field()
        self.assertThat(text_field.text, Eventually(Contains(text)))

    def go_to_url(self, url):
        self.ensure_chrome_is_hidden()
        self.reveal_chrome()
        self.clear_address_bar()
        self.type_in_address_bar(url)
        self.keyboard.press_and_release("Enter")

    def assert_page_eventually_loading(self):
        webview = self.main_window.get_current_webview()
        self.assertThat(webview.loading, Eventually(Equals(True)))

    def assert_page_eventually_loaded(self, url):
        webview = self.main_window.get_current_webview()
        self.assertThat(webview.url, Eventually(Equals(url)))
        self.assertThat(webview.loading, Eventually(Equals(False)))

    def open_html_page(self):
        title = "start page"
        body = "<p>Lorem ipsum dolor sit amet.</p>"
        self.url = self.make_html_page(title, body)
        address_bar = self.main_window.get_address_bar()
        self.pointing_device.click_object(address_bar)
        self.keyboard.type(self.url, delay=TYPING_DELAY)
        self.keyboard.press_and_release("Enter")
        self.assert_home_page_eventually_loaded()

    def assert_home_page_eventually_loaded(self):
        self.assert_page_eventually_loaded(self.url)

    def open_remote_page(self):
        self.base_url = "http://localhost:%d" % HTTP_SERVER_PORT
        self.url = self.base_url + "/loremipsum"
        address_bar = self.main_window.get_address_bar()
        self.pointing_device.click_object(address_bar)
        self.keyboard.type(self.url, delay=TYPING_DELAY)
        self.keyboard.press_and_release("Enter")
        self.assert_home_page_eventually_loaded()

    def assert_home_page_eventually_loaded(self):
        self.assert_page_eventually_loaded(self.url)


class StartOpenLocalPageTestCaseBase(BrowserTestCaseBase):

    """Helper test class that opens the browser at a local URL instead of
    defaulting to the homepage."""

    def setUp(self):
        title = "start page"
        body = "<p>Lorem ipsum dolor sit amet.</p>"
        self.url = self.make_html_page(title, body)
        self.ARGS = [self.url]
        super(StartOpenLocalPageTestCaseBase, self).setUp()
        self.assert_home_page_eventually_loaded()

    def assert_home_page_eventually_loaded(self):
        self.assert_page_eventually_loaded(self.url)


class BrowserTestCaseBaseWithHTTPServer(BrowserTestCaseBase):

    """
    A specialization of the common test case class that runs
    a simple custom HTTP server in a separate thread.
    """

    def setUp(self):
        self.server = http_server.HTTPServerInAThread(HTTP_SERVER_PORT)
        self.server.start()
        super(BrowserTestCaseBaseWithHTTPServer, self).setUp()

    def tearDown(self):
        super(BrowserTestCaseBaseWithHTTPServer, self).tearDown()
        self.server.shutdown()

    def type_url(self, url):
        address_bar = self.main_window.get_address_bar()
        self.pointing_device.click_object(address_bar)
        self.keyboard.type(self.url, delay=TYPING_DELAY)
        self.keyboard.press_and_release("Enter")

    def assert_home_page_eventually_loaded(self):
        self.assert_page_eventually_loaded(self.url)


class StartOpenRemotePageTestCaseBase(BrowserTestCaseBaseWithHTTPServer):

    """
    Helper test class that opens the browser at a remote URL instead of
    defaulting to the homepage.

    This class should be preferred to the base test case class, as it doesn’t
    rely on a connection to the outside world (to open the default homepage),
    and because it ensures the initial page is fully loaded before the tests
    are executed, thus making them more robust.
    """

    def setUp(self):
        super(StartOpenRemotePageTestCaseBase, self).setUp()
        self.base_url = "http://localhost:%d" % HTTP_SERVER_PORT
        self.url = self.base_url + "/loremipsum"
        self.type_url(self.url)
        self.assert_home_page_eventually_loaded()

    def assert_home_page_eventually_loaded(self):
        self.assert_page_eventually_loaded(self.url)
