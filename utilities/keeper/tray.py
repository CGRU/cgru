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
      action = QtGui.QAction('Render Info', self)
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
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.exitRender)
      self.menu['AFANASY'].addAction( action)
      action = QtGui.QAction('Exit Watch', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.exitMonitor)
      self.menu['AFANASY'].addAction( action)
      action = QtGui.QAction('Exit Talk', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.exitTalk)
      self.menu['AFANASY'].addAction( action)
      action = QtGui.QAction('Exit All Clients', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.exitClients)
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
      action = QtGui.QAction('Quit && Exit Clients', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.quitExitClients)
      self.menu['menu'].addAction( action)            
      action = QtGui.QAction('Quit', self)
      QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.quit)
      self.menu['menu'].addAction( action)            

      self.setContextMenu( self.menu['menu'])

      # Prepare Icons:
      self.icons = dict()
      icon_filename = cgruconfig.VARS['tray_icon']
      if icon_filename is None: icon_filename = 'keeper'
      icon_filename = os.path.join( cgruconfig.VARS['icons_dir'], icon_filename + '.png')
      self.icon_epmty = QtGui.QPixmap( icon_filename)
      self.icons['empty'] = QtGui.QIcon( self.icon_epmty)
      self.makeIcon('offline_free',       online=False, nimby=False, busy=False)
      self.makeIcon('online_free',        online=True,  nimby=False, busy=False)
      self.makeIcon('offline_nimby',      online=False, nimby=True,  busy=False)
      self.makeIcon('online_nimby',       online=True,  nimby=True,  busy=False)
      self.makeIcon('offline_free_busy',  online=False, nimby=False, busy=True )
      self.makeIcon('online_free_busy',   online=True,  nimby=False, busy=True )
      self.makeIcon('offline_nimby_busy', online=False, nimby=True,  busy=True )
      self.makeIcon('online_nimby_busy',  online=True,  nimby=True,  busy=True )

      # Decorate and show:
      self.showIcon()
      self.setToolTip( cgruconfig.VARS['company'].upper() + ' Keeper ' + os.getenv('CGRU_VERSION', ''))
      QtCore.QObject.connect( self, QtCore.SIGNAL('activated( QSystemTrayIcon::ActivationReason)'), self.activated_slot)

      self.show()

   def makeIcon( self, name, online, nimby, busy):
      painting = self.icon_epmty
      painter = QtGui.QPainter( painting)
      icon_size = painting.width()
      text_font = QtGui.QFont('Arial',icon_size/3)
      text_font.setBold( True)
      rect_back = QtCore.QRect( icon_size*3/10, icon_size*3/10, icon_size*2/5, icon_size*2/5)
      text_color = QtGui.QColor( 0, 0, 0)
      if online:  text_color = QtGui.QColor(   0, 200,   0)
      if busy:    text_color = QtGui.QColor( 255,   0,   0)
      elif nimby: text_color = QtGui.QColor( 200, 200, 200)
      if nimby:   back_color = QtGui.QColor(  40,  40, 240)
      else:       back_color = QtGui.QColor( 150, 150, 150)
      rect_render = QtCore.QRect( icon_size/4, icon_size/4, icon_size/2, icon_size/2)      
      painter.fillRect( rect_back, back_color)
      painter.setFont( text_font)
      painter.setPen( text_color)
      painter.drawText( rect_render, QtCore.Qt.AlignCenter,'R')
      self.icons[name] = QtGui.QIcon( painting)

   def showIcon( self, name = 'empty'):
      self.setIcon( self.icons[ name])
      self.parent.setWindowIcon( self.icons[ name])

   def showRenderIcon( self, online, nimby, busy):
      if online: name = 'online'
      else: name = 'offline'
      if nimby: name += '_nimby'
      else: name += '_free'
      if busy: name += '_busy'
      self.showIcon( name)

   def activated_slot( self, reason):
      if reason == QtGui.QSystemTrayIcon.Trigger: return
      elif reason == QtGui.QSystemTrayIcon.DoubleClick: render.refresh()
      elif reason == QtGui.QSystemTrayIcon.MiddleClick: return 
      elif reason == QtGui.QSystemTrayIcon.Context: return 
      elif reason == QtGui.QSystemTrayIcon.Unknown: return

   def renderInfo( self): render.showInfo( self)
   def cgruInfo( self): self.dialog_info = info.Window()
   def editNimby( self): self.dialog_nimby = DialogNimby()
