import os, sys

import cgrudocs
import cgruconfig

from PyQt4 import QtCore, QtGui

class Tray( QtGui.QSystemTrayIcon):
   def __init__( self, parent = None):
      QtCore.QObject.__init__( self, parent)

      self.menu = QtGui.QMenu()
      quitAction = QtGui.QAction('Edit Nimby...', self)
      QtCore.QObject.connect( quitAction, QtCore.SIGNAL('triggered()'), self.editNimby)
      self.menu.addAction( quitAction)
      quitAction = QtGui.QAction('Edit CRGU Config...', self)
      QtCore.QObject.connect( quitAction, QtCore.SIGNAL('triggered()'), self.editCGRUConfig)
      self.menu.addAction( quitAction)
      self.menu.addSeparator()
      quitAction = QtGui.QAction('CGRU Documentation...', self)
      QtCore.QObject.connect( quitAction, QtCore.SIGNAL('triggered()'), self.cgruDocs)
      self.menu.addAction( quitAction)
      self.menu.addSeparator()
      quitAction = QtGui.QAction('Quit', self)
      QtCore.QObject.connect( quitAction, QtCore.SIGNAL('triggered()'), parent.quit)
      self.menu.addAction( quitAction)
      self.setContextMenu( self.menu)

      self.setIcon( QtGui.QIcon( os.path.join( os.path.join( os.getenv('CGRU_KEEPER', ''), 'icons'), 'keeper.svg')))
      self.setToolTip('CGRU Keeper ' + os.getenv('CGRU_VERSION', ''))

      self.show()

   def editCGRUConfig( self):
      print('editCGRUConfig( self): START:')
      config = cgruconfig.VARS['HOME_CONFIGFILE']
      edit = 'gedit'
      if sys.platform.find('win') == 0: edit == 'notepad'
      print('editCGRUConfig( self): EXEC:')
#      QtCore.QProcess.startDetached( edit + ' ' + config)
      if QtCore.QProcess.execute( edit + ' ' + config) == 0: cgruconfig.Config()
      print('editCGRUConfig( self): END')

   def cgruDocs( self):
      cgrudocs.show()

   def editNimby( self):
      print('edit Nimby !')
