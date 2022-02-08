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


    def setInterval(self, i_sec):
        msec = i_sec * 1000
        if self.timer.interval() != msec:
            print('Setting refresh interval to %d seconds.' % i_sec)
            self.timer.setInterval(msec)


    def setDefaultInterval(self):
        sec = int(cgruconfig.VARS['keeper_refresh'])
        self.setInterval(sec)


    def setIntervalKoeff(self, i_koeff):
        sec = i_koeff * int(cgruconfig.VARS['keeper_refresh'])
        self.setInterval(sec)


    def refresh(self):
        if self.counter == 1:
            self.setDefaultInterval()

        nimby.refresh()
        render.refresh()
        self.counter += 1
