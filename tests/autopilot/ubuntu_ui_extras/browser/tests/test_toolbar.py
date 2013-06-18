# -*- coding: utf-8 -*-
#
# Copyright 2013 Canonical
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.

from __future__ import absolute_import

from autopilot.matchers import Eventually
from testtools.matchers import Equals

from ubuntu_ui_extras.browser.tests import BrowserTestCaseBase


class TestToolbar(BrowserTestCaseBase):

    """Tests interaction with the toolbar."""

    def test_unfocus_chrome_hides_it(self):
        webview = self.main_window.get_web_view()
        panel = self.main_window.get_panel()
        self.ensure_chrome_is_hidden()
        self.reveal_chrome()
        self.pointing_device.click_object(webview)
        self.assertThat(panel.state, Eventually(Equals("")))
