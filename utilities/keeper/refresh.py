# -*- coding: utf-8 -*-

import threading

import cgruconfig
import nimby
import render

from Qt import QtCore

# Set a default refresh interval in seconds:
if 'keeper_refresh' not in cgruconfig.VARS:
    cgruconfig.VARS['keeper_refresh'] = '10'


class Refresh(QtCore.QObject):
    # Worker -> UI (QueuedConnection). Plain QTimer.singleShot from a Python
    # thread is unreliable on PySide6 and can leave the tray icon stuck.
    _network_done = QtCore.Signal(object)
    _schedule = QtCore.Signal(int)

    def __init__(self, application):
        QtCore.QObject.__init__(self, application)
        self.app = application
        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self.refresh)
        self.timer.setInterval(1000 * 3)
        self.timer.start()
        self.counter = 0
        self._busy = False
        self._pending = False
        self._network_done.connect(self._apply)
        self._schedule.connect(self._onSchedule)
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

    def scheduleRefresh(self, i_sec=3):
        """Request a refresh after i_sec. Safe to call from any thread."""
        self._schedule.emit(int(i_sec))

    def _onSchedule(self, i_sec):
        QtCore.QTimer.singleShot(1000 * i_sec, self.refresh)

    def refresh(self):
        # Network must not run on the UI thread: failed DNS to af_servername
        # used to freeze the tray menu for ~10 seconds.
        if self._busy:
            self._pending = True
            return

        if self.counter == 1:
            self.setDefaultInterval()

        self._busy = True
        self._pending = False
        self.counter += 1

        def work():
            interval = None
            try:
                nimby.refresh()
                interval = render.refreshNetwork()
            except Exception as err:
                print('Keeper refresh error: %s' % err)
            self._network_done.emit(interval)

        threading.Thread(target=work, name='keeper-refresh', daemon=True).start()

    def _apply(self, interval):
        try:
            render.applyRefreshInterval(interval)
            render.refreshUI()
        finally:
            self._busy = False
            if self._pending:
                self._pending = False
                self.refresh()
