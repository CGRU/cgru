import os, sys
import subprocess

import cgrudocs
import cgruconfig

import nimby

from dialog_nimby import DialogNimby

from PyQt4 import QtCore, QtGui

def getVar( var, title = 'Set Variable', label = 'Enter new value:'):
   oldvalue = ''
   if var in cgruconfig.VARS: oldvalue = cgruconfig.VARS[var]
   newvalue, ok = QtGui.QInputDialog.getText( None, title, label, text = oldvalue)
   if not ok: return
   cgruconfig.VARS[var] = newvalue
   variables = [var]
   cgruconfig.writeVars(variables)

class Tray( QtGui.QSystemTrayIcon):
   def __init__( self, parent = None):
      QtGui.QSystemTrayIcon.__init__( self, parent)

      # Menu:
      self.menu = QtGui.QMenu()

      self.menu_af = QtGui.QMenu('AFANASY')

      action = QtGui.QAction('Set nibmy', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), nimby.setnimby)
      self.menu_af.addAction( action)

      action = QtGui.QAction('Set NIMBY', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), nimby.setNIMBY)
      self.menu_af.addAction( action)

      action = QtGui.QAction('Set Free', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), nimby.setFree)
      self.menu_af.addAction( action)

      action = QtGui.QAction('Nimby Schedule...', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.editNimby)
      self.menu_af.addAction( action)

      self.menu.addMenu( self.menu_af)

      self.menu.addSeparator()

      self.menu_conf = QtGui.QMenu('Configure...')

      action = QtGui.QAction('Reload', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.confReload)
      self.menu_conf.addAction( action)

      action = QtGui.QAction('Set Docs URL...', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.setDocsURL)
      self.menu_conf.addAction( action)

      action = QtGui.QAction('Edit CRGU Config...', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.editCGRUConfig)
      self.menu_conf.addAction( action)

      action = QtGui.QAction('Set Text Editor...', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.setTextEditor)
      self.menu_conf.addAction( action)

      self.menu.addMenu( self.menu_conf)

      self.menu.addSeparator()

      action = QtGui.QAction('CGRU Documentation...', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.cgruDocs)
      self.menu.addAction( action)

      self.menu.addSeparator()

      action = QtGui.QAction('Quit', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), parent.quit)
      self.menu.addAction( action)

      # Decorate and show:
      self.setContextMenu( self.menu)
      self.icon = QtGui.QIcon( os.path.join( os.path.join( os.getenv('CGRU_KEEPER', ''), 'icons'), 'keeper.png'))
      self.setIcon( self.icon)
      parent.setWindowIcon( self.icon)
   
      self.setToolTip('CGRU Keeper ' + os.getenv('CGRU_VERSION', ''))

      self.show()

   def confReload( self): cgruconfig.Config()
   def cgruDocs( self): cgrudocs.show()
   def editNimby( self): self.dialog_nimby = DialogNimby()
   def setDocsURL( self): getVar('docshost','Set Docs Host','Enter host name or IP address:')
   def setTextEditor( self): getVar('editor','Set Text Editor','Enter command with "%s":')

   def editCGRUConfig( self):
      if QtCore.QProcess.execute( cgruconfig.VARS['editor'] % cgruconfig.VARS['HOME_CONFIGFILE']) == 0:
         cgruconfig.Config()
