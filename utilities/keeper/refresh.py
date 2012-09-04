import cgruconfig
import nimby
import render

from cgrupyqt import QtCore

# Set a default refresh interval in seconds:
if 'keeper_refresh' not in cgruconfig.VARS: cgruconfig.VARS['keeper_refresh'] = '36'

class Refresh:
   def __init__( self, application):
      self.timer = QtCore.QTimer( application)
      QtCore.QObject.connect( self.timer, QtCore.SIGNAL('timeout()'), self.refresh)
      self.timer.setInterval( 1000 * 3)
      self.timer.start()
      self.counter = 0
      self.refresh()

   def refresh( self):
      if self.counter > 0:
         cgruconfig.Config()
         if self.timer.interval() != 1000 * int(cgruconfig.VARS['keeper_refresh']):
            self.timer.setInterval( 1000 * int(cgruconfig.VARS['keeper_refresh']))
      nimby.refresh()
      render.refresh()
      self.counter += 1
