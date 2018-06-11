# -*- coding: utf-8 -*-

import cgruconfig
import nimby
import render

from Qt import QtCore

# Set a default refresh interval in seconds:
if 'keeper_refresh' not in cgruconfig.VARS:
    cgruconfig.VARS['keeper_refresh'] = '10'


class Refresh:
    def __init__(self, application):
        self.timer = QtCore.QTimer( application)
        self.timer.timeout.connect( self.refresh)
        self.timer.setInterval(1000 * 3)
        self.timer.start()
        self.counter = 0
        self.refresh()

    def refresh(self):
        if self.counter > 0:
            keeper_refresh = 1000 * int(cgruconfig.VARS['keeper_refresh'])
            if self.timer.interval() != keeper_refresh:
                self.timer.setInterval(keeper_refresh)
        nimby.refresh()
        render.refresh()
        self.counter += 1
