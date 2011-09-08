import cgruconfig
import nimby

from PyQt4 import QtCore

class Refresh:
   def __init__( self, application):
      self.timer = QtCore.QTimer( application)
      QtCore.QObject.connect( self.timer, QtCore.SIGNAL('timeout()'), self.refresh)
      self.timer.setInterval( 1000 * int(cgruconfig.VARS['keeper_refresh']))
      self.timer.start()
      self.counter = 0
      self.refresh()

   def refresh( self):
      if self.counter > 0: cgruconfig.Config()
      if self.timer.interval() != 1000 * int(cgruconfig.VARS['keeper_refresh']):
         self.timer.setInterval( 1000 * int(cgruconfig.VARS['keeper_refresh']))
      nimby.refresh()
      self.counter += 1
