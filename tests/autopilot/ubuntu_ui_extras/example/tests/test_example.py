# -*- coding: utf-8 -*-
#
# Copyright 2013 Canonical
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.

from __future__ import absolute_import

from testtools.matchers import Equals
from autopilot.matchers import Eventually

from ubuntu_ui_extras.example.tests import ExampleTestCaseBase


class TestTabs(ExampleTestCaseBase):

    """Tests tabs management."""

    def test_ensure_something(self):
        self.assertThat(True, Equals(True))
        # Add your own real tests here
