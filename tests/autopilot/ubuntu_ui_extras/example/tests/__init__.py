# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2013 Canonical
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.

"""ubuntu-ui-extras autopilot tests."""

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

from ubuntu_ui_extras.example.emulators.main_window import MainWindow


class ExampleTestCaseBase(AutopilotTestCase):

    """
    A common test case class that provides several useful methods
    for the component tests.
    """

    if model() == 'Desktop':
        scenarios = [('with mouse', dict(input_device_class=Mouse)), ]
    else:
        scenarios = [('with touch', dict(input_device_class=Touch)), ]

    ARGS = []
    _temp_pages = []
    local_location = "ubuntu_ui_extras/example/emulators/runner.qml"

    def setUp(self):
        self.pointing_device = Pointer(self.input_device_class.create())
        super(ExampleTestCaseBase, self).setUp()

        if os.path.exists(self.local_location):
            self.launch_test_local()
        else:
            self.launch_test_installed()

    def tearDown(self):
        super(ExampleTestCaseBase, self).tearDown()
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
        runner = os.path.join(get_python_lib(), self.local_location)

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
