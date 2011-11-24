import os, sys
import subprocess

import cgruconfig

import af

import cmd
import info
import nimby
import render
import software

from dialog_nimby import DialogNimby

from PyQt4 import QtCore, QtGui

class ActionCommand( QtGui.QAction):
   def __init__( self, parent, name, command, iconpath = None):
      QtGui.QAction.__init__( self, name, parent)
      if iconpath is not None:
         iconpath = os.path.join( cgruconfig.VARS['icons_dir'], iconpath)
         if os.path.isfile( iconpath):
            self.setIcon( QtGui.QIcon( iconpath))
         else:
            print('WARNING: Icon "%s" does not exist.' % iconpath)
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
         QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.update)
         self.menu['menu'].addAction( action)
         self.menu['menu'].addSeparator()

      # Software menu:
      self.menu['Software'] = QtGui.QMenu('Software')
      self.menu['menu'].addMenu( self.menu['Software'])
      action = QtGui.QAction( QtGui.QIcon( cgruconfig.VARS['icons_dir'] + '/folder.png'), '[ browse ]', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), software.browse)
      self.menu['Software'].addAction( action)
      for soft in software.Names:
         icon = software.getIcon( soft)
         if icon is None:
            action = QtGui.QAction( soft, self)
         else:
            action = QtGui.QAction( icon, soft, self)
         eval("QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), software.start%s)" % soft)
         self.menu['Software'].addAction( action)
      # Software setup:
      self.menu['Setup Soft'] = QtGui.QMenu('Setup Soft')
      self.menu['Software'].addMenu( self.menu['Setup Soft'])
      for soft in software.Names:
         action = QtGui.QAction( soft, self)
         eval("QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), software.locate%s)" % soft)
         self.menu['Setup Soft'].addAction( action)
      # Software examples:
      self.menu['Examples'] = QtGui.QMenu('Examples')
      self.menu['Software'].addMenu( self.menu['Examples'])
      for soft in software.Names:
         action = QtGui.QAction( soft, self)
         eval("QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), software.example%s)" % soft)
         self.menu['Examples'].addAction( action)
         
      # Load menu:
      for dirpath, dirnames, filenames in os.walk( cgruconfig.VARS['menu'], True, None, True):
         if dirpath.find('/.') != -1: continue
         if dirpath.find('\\.') != -1: continue
         menuname = os.path.basename( dirpath)
         if menuname == os.path.basename( cgruconfig.VARS['menu']):
            menuname = 'menu'
         else:
            iconpath = os.path.join( cgruconfig.VARS['icons_dir'], menuname.lower() + '.png')
            if os.path.isfile( iconpath):
               self.menu[menuname] = self.menu['menu'].addMenu( QtGui.QIcon( iconpath), menuname)
            else:
               self.menu[menuname] = self.menu['menu'].addMenu( menuname)
         filenames.sort()
         for filename in filenames:
            if filename[0] == '.' or filename[0] == '_': continue
            if sys.platform[:3] == 'win':
               if filename[-4:] != '.cmd': continue
               itemname = filename[:-4]
            else:
               if filename[-3:] != '.sh': continue
               itemname = filename[:-3]
            filename = os.path.join( dirpath, filename)
            file = open( filename,'r')
            lines = file.readlines()
            file.close()
            iconpath = None
            for line in lines:
               if line.find('Name=') != -1:
                  itemname = line.split('Name=')[-1].strip()
               if line.find('Icon=') != -1:
                  iconpath = line.split('Icon=')[-1].strip()
            action = ActionCommand( self, itemname, filename, iconpath)
            self.menu[menuname].addAction( action)
            QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), action.runCommand)

      # Add permanent items to 'Afanasy':
      if not 'AFANASY' in self.menu:
         self.menu['AFANASY'] = QtGui.QMenu('AFANASY')
         self.menu['menu'].addMenu( self.menu['AFANASY'])
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

      action = QtGui.QAction('Set Server...', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.setAFANASYServer)
      self.menu['AFANASY'].addAction( action)
      action = QtGui.QAction('Edit User Config...', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.editAFANASYConfig)
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

      # Add permanent items to 'Configure':
      if not 'Configure' in self.menu:
         self.menu['Configure'] = QtGui.QMenu('Configure')
         self.menu['menu'].addMenu( self.menu['Configure'])
      self.menu['Configure'].addSeparator()
      action = QtGui.QAction('Reload Config', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.confReload)
      self.menu['Configure'].addAction( action)
      action = QtGui.QAction('Set Docs URL...', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.setDocsURL)
      self.menu['Configure'].addAction( action)
      action = QtGui.QAction('Edit Config...', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.editCGRUConfig)
      self.menu['Configure'].addAction( action)
      action = QtGui.QAction('Set Text Editor...', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.setTextEditor)
      self.menu['Configure'].addAction( action)

      self.menu['menu'].addSeparator()

      action = QtGui.QAction('Show Info', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), self.cgruInfo)
      self.menu['menu'].addAction( action)
      self.menu['menu'].addSeparator()
      action = QtGui.QAction('Documentation', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.cgruDocs)
      self.menu['menu'].addAction( action)
      self.menu['menu'].addSeparator()
      action = QtGui.QAction('Restart', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.restart)
      self.menu['menu'].addAction( action)            
      action = QtGui.QAction('Quit', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.quit)
      self.menu['menu'].addAction( action)            

      # Decorate and show:
      self.icon_name = cgruconfig.VARS['tray_icon']
      if self.icon_name is None: self.icon_name = 'keeper'
      self.setContextMenu( self.menu['menu'])
      self.icon = QtGui.QIcon( os.path.join( cgruconfig.VARS['icons_dir'], self.icon_name + '.png'))
      self.setIcon( self.icon)
      parent.setWindowIcon( self.icon)
      self.setToolTip( cgruconfig.VARS['company'].upper() + ' Keeper ' + os.getenv('CGRU_VERSION', ''))
      QtCore.QObject.connect( self, QtCore.SIGNAL('activated( QSystemTrayIcon::ActivationReason)'), self.activated_slot)

      self.show()

   def activated_slot( self, reason):
      if reason == QtGui.QSystemTrayIcon.Trigger: return
      elif reason == QtGui.QSystemTrayIcon.DoubleClick:
#         render.refresh()
         print('DoubleClick')
      elif reason == QtGui.QSystemTrayIcon.MiddleClick:
         print('MiddleClick')
      elif reason == QtGui.QSystemTrayIcon.Context: return 
      elif reason == QtGui.QSystemTrayIcon.Unknown: return

   def renderInfo( self): render.showInfo( self)
   def cgruInfo( self): self.dialog_info = info.Window()
   def editNimby( self): self.dialog_nimby = DialogNimby()
