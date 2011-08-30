import os, sys

# Write qt.conf file:
if sys.platform.find('win') == 0:
   qtconf=os.getenv('PYTHON') + '/qt.conf'
   pyqt4dir=os.getenv('PYTHON').replace('\\','/') + '/Lib/site-packages/PyQt4'
   if os.path.isfile( qtconf): os.remove( qtconf)
   qtconf_file = open(qtconf, mode='w')
   print('[Paths]', file=qtconf_file)
   print('Prefix = ' + pyqt4dir, file=qtconf_file)
   print('Binaries = ' + pyqt4dir, file=qtconf_file)
   qtconf_file.close()

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
