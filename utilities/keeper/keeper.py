import os, sys

from PyQt4 import QtCore, QtGui

class Dialog( QtGui.QWidget):
   def __init__( self, parent = None):
      QtGui.QWidget.__init__( self, parent)
      self.setWindowTitle('CGRU Keeper ' + os.getenv('CGRU_VERSION', ''))
      self.mainLayout = QtGui.QVBoxLayout( self)

class Tray( QtGui.QSystemTrayIcon):
   def __init__( self, parent = None):
      QtCore.QObject.__init__( self, parent)

      quitAction = QtGui.QAction('Quit', self)
      QtCore.QObject.connect( quitAction, QtCore.SIGNAL('triggered()'), app.quit)

      self.menu = QtGui.QMenu()
      self.menu.addSeparator()
      self.menu.addAction( quitAction)
      self.menu.addSeparator()
      self.setContextMenu( self.menu)

      self.setIcon( QtGui.QIcon( os.path.join( os.path.join( os.getenv('CGRU_KEEPER', ''), 'icons'), 'keeper.svg')))
      self.setToolTip('CRGU Keeper')

      self.show()

app = QtGui.QApplication( sys.argv)
icon = QtGui.QIcon( os.path.join( os.path.join( os.getenv('CGRU_KEEPER', ''), 'icons'), 'keeper.svg'))
app.setWindowIcon( icon)
tray = Tray( app)
dialog = Dialog()
dialog.show()
app.exec_()

