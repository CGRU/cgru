import os, sys
import subprocess

import cgrudocs
import cgruconfig

import af

import info
import nimby
import render

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

class ActionCommand( QtGui.QAction):
   def __init__( self, parent, name, command):
      QtGui.QAction.__init__( self, name, parent)
      self.name = name
      self.cmd = command
   def runCommand( self): QtCore.QProcess.startDetached( self.cmd, [])

class Tray( QtGui.QSystemTrayIcon):
   def __init__( self, parent = None):
      QtGui.QSystemTrayIcon.__init__( self, parent)
      self.parent = parent

      # Menu:
      self.menu = dict()
      self.menu['menu'] = QtGui.QMenu()

      # Update item only if CGRU_UPDATE_CMD defined:
      if cgruconfig.VARS['CGRU_UPDATE_CMD'] is not None:
         action = QtGui.QAction('Update', self)
         QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.update)
         self.menu['menu'].addAction( action)
         self.menu['menu'].addSeparator()

      # Software menu:
      self.menu['Software'] = QtGui.QMenu('Software')
      self.menu['menu'].addMenu( self.menu['Software'])
      action = QtGui.QAction('Nuke', self)
#      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.update)
      self.menu['Software'].addAction( action)
      self.menu['Setup Soft'] = QtGui.QMenu('Setup Soft')
      self.menu['Software'].addMenu( self.menu['Setup Soft'])
         
      # Load menu:
      menudir = os.path.join( os.environ['CGRU_KEEPER'], 'menu')
      for dirpath, dirnames, filenames in os.walk( menudir, True, None, True):
         if dirpath.find('/.') != -1: continue
         if dirpath.find('\\.') != -1: continue
         menuname = os.path.basename( dirpath)
         if menuname != 'menu':
            self.menu[menuname] = QtGui.QMenu( menuname)
            self.menu['menu'].addMenu( self.menu[menuname])
         filenames.sort()
         for filename in filenames:
            if filename[0] == '.': continue
            if sys.platform[:3] == 'win':
               if filename[-4:] != '.cmd': continue
               itemname = filename[:-4]
            else:
               if filename[-3:] != '.sh': continue
               itemname = filename[:-3]
            action = ActionCommand( self, itemname, os.path.join( dirpath, filename))
            self.menu[menuname].addAction( action)
            QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), action.runCommand)

      # Add permanent items:
      self.menu['AFANASY'].addSeparator()
      action = QtGui.QAction('Set nibmy', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), nimby.setnimby)
      self.menu['AFANASY'].addAction( action)
      action = QtGui.QAction('Set NIMBY', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), nimby.setNIMBY)
      self.menu['AFANASY'].addAction( action)
      action = QtGui.QAction('Set Free', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), nimby.setFree)
      self.menu['AFANASY'].addAction( action)
      action = QtGui.QAction('Eject Tasks', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), nimby.ejectTasks)
      self.menu['AFANASY'].addAction( action)
      action = QtGui.QAction('Render info', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.renderInfo)
      self.menu['AFANASY'].addAction( action)

      self.menu['AFANASY'].addSeparator()

      action = QtGui.QAction('Nimby Schedule...', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.editNimby)
      self.menu['AFANASY'].addAction( action)

      self.menu['AFANASY'].addSeparator()

      action = QtGui.QAction('Exit Render', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), render.exit)
      self.menu['AFANASY'].addAction( action)
      action = QtGui.QAction('Exit Watch', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), render.exitmonitor)
      self.menu['AFANASY'].addAction( action)
      action = QtGui.QAction('Exit Talk', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), render.exittalk)
      self.menu['AFANASY'].addAction( action)

      self.menu['menu'].addSeparator()

      self.menu['Configure'].addSeparator()
      action = QtGui.QAction('Reload Config', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.confReload)
      self.menu['Configure'].addAction( action)
      action = QtGui.QAction('Set Docs URL...', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.setDocsURL)
      self.menu['Configure'].addAction( action)
      action = QtGui.QAction('Edit Config...', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.editCGRUConfig)
      self.menu['Configure'].addAction( action)
      action = QtGui.QAction('Set Text Editor...', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.setTextEditor)
      self.menu['Configure'].addAction( action)

      self.menu['menu'].addSeparator()

      action = QtGui.QAction('Show Info', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.cgruInfo)
      self.menu['menu'].addAction( action)
      self.menu['menu'].addSeparator()
      action = QtGui.QAction('Documentation', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.cgruDocs)
      self.menu['menu'].addAction( action)
      self.menu['menu'].addSeparator()
      action = QtGui.QAction('Restart', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.restart)
      self.menu['menu'].addAction( action)            
      action = QtGui.QAction('Quit', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), parent.quit)
      self.menu['menu'].addAction( action)            

      # Decorate and show:
      self.icon_name = cgruconfig.VARS['tray_icon']
      if self.icon_name is None: self.icon_name = 'keeper'
      self.setContextMenu( self.menu['menu'])
      self.icon = QtGui.QIcon( os.path.join( os.path.join( os.getenv('CGRU_KEEPER', ''), 'icons'), self.icon_name + '.png'))
      self.setIcon( self.icon)
      parent.setWindowIcon( self.icon)
      self.setToolTip( cgruconfig.VARS['company'].upper() + ' Keeper ' + os.getenv('CGRU_VERSION', ''))
      QtCore.QObject.connect( self, QtCore.SIGNAL('activated( QSystemTrayIcon::ActivationReason)'), self.activated_slot)

      self.show()

   def activated_slot( self, reason):
      if reason == QtGui.QSystemTrayIcon.Trigger: return
      elif reason == QtGui.QSystemTrayIcon.DoubleClick:
         render.refresh()
         print('DoubleClick')
      elif reason == QtGui.QSystemTrayIcon.MiddleClick:
         print('MiddleClick')
      elif reason == QtGui.QSystemTrayIcon.Context: return 
      elif reason == QtGui.QSystemTrayIcon.Unknown: return

   def renderInfo( self): render.showInfo( self)
   def confReload( self): cgruconfig.Config()
   def cgruDocs( self): cgrudocs.show()
   def cgruInfo( self): self.dialog_info = info.Window()
   def editNimby( self): self.dialog_nimby = DialogNimby()
   def setDocsURL( self): getVar('docshost','Set Docs Host','Enter host name or IP address:')
   def setTextEditor( self): getVar('editor','Set Text Editor','Enter command with "%s":')
   def startAfWatch( self): QtCore.QProcess.startDetached( os.path.join( os.path.join( os.getenv('AF_ROOT'), 'launch'), 'afwatch.sh'))

   def restart( self):
      QtCore.QProcess.startDetached( cgruconfig.VARS['CGRU_KEEPER_CMD'])
      self.parent.quit()

   def update( self):
      QtCore.QProcess.startDetached( cgruconfig.VARS['CGRU_UPDATE_CMD'])
      self.parent.quit()

   def editCGRUConfig( self):
      if QtCore.QProcess.execute( cgruconfig.VARS['editor'] % cgruconfig.VARS['HOME_CONFIGFILE']) == 0:
         cgruconfig.Config()
