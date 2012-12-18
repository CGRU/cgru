import os, sys
import subprocess

import cgruconfig
import cgruutils

import af

import cmd
import info
import nimby
import render
import software

from nimby_dialog import NimbyDialog

from cgrupyqt import QtCore, QtGui

class ActionCommand( QtGui.QAction):
	def __init__( self, parent, name, command, iconpath = None):
		QtGui.QAction.__init__( self, name, parent)
		if iconpath is not None:
			iconpath = cgruutils.getIconFileName( iconpath)
			if iconpath is not None:
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

		# Load menu:
		menu_path = os.path.join( os.path.join( cgruconfig.VARS['CGRU_LOCATION'],'start'))
		menu_paths = cgruconfig.VARS['menu_path']
		if menu_paths is None: menu_paths = menu_path
		if menu_paths.find(';') != -1: menu_paths = menu_paths.split(';')
		elif sys.platform.find('win') == -1: menu_paths = menu_paths.split(':')
		else: menu_paths = [menu_paths]
		if not menu_path in menu_paths: menu_paths.append( menu_path)
		for menu_path in menu_paths:
			if not os.path.isdir( menu_path): continue
			for dirpath, dirnames, filenames in os.walk( menu_path, True, None, True):
				if dirpath.find('/.') != -1: continue
				if dirpath.find('\\.') != -1: continue
				menuname = os.path.basename( dirpath)
				if menuname == os.path.basename( menu_path):
					menuname = 'menu'
				else:
					self.addMenu( self.menu['menu'], menuname)
				filenames.sort()
				was_separator = True
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
					separator = False
					for line in lines:
						if line.find('Name=') != -1:
							itemname = line.split('Name=')[-1].strip()
						if line.find('Icon=') != -1:
							iconpath = line.split('Icon=')[-1].strip()
						if line.find('Separator') != -1: separator = True
					if separator:
						if not was_separator: self.menu[menuname].addSeparator()
						was_separator = True
					else: was_separator = False
					action = ActionCommand( self, itemname, filename, iconpath)
					self.menu[menuname].addAction( action)
					QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), action.runCommand)
					if separator: self.menu[menuname].addSeparator()

		# Add permanent items to 'Afanasy':
		if not self.addMenu( self.menu['menu'], 'AFANASY'): self.menu['AFANASY'].addSeparator()

		action = QtGui.QAction('Web GUI', self)
		QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.afwebgui)
		self.menu['AFANASY'].addAction( action)
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
		action = QtGui.QAction('Eject Not My Tasks', self)
		QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), nimby.ejectNotMyTasks)
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

		# Add Software menu if it was not created by custom files:
		if not 'Software' in self.menu:
			self.addMenu( self.menu['menu'], 'Software')
			self.menu['menu'].addSeparator()
			action = QtGui.QAction( QtGui.QIcon( cgruutils.getIconFileName('folder')), '[ browse ]', self)
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

		# Add permanent items to 'Configure':
		if not self.addMenu( self.menu['menu'], 'Configure'): self.menu['Configure'].addSeparator()
		action = QtGui.QAction('Reload Config', self)
		QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.confReload)
		self.menu['Configure'].addAction( action)
		action = QtGui.QAction('Set Docs URL...', self)
		QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.setDocsURL)
		self.menu['Configure'].addAction( action)
		action = QtGui.QAction('Edit Config...', self)
		QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.editCGRUConfig)
		self.menu['Configure'].addAction( action)
		action = QtGui.QAction('Set Web Browser...', self)
		QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.setWebBrowser)
		self.menu['Configure'].addAction( action)
		action = QtGui.QAction('Set Text Editor...', self)
		QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), cmd.setTextEditor)
		self.menu['Configure'].addAction( action)

		self.addAction('menu', True,'Show Info...', self.cgruInfo,'info')
		self.addAction('menu', True,'Documentation', cmd.cgruDocs)
		self.addAction('menu', True,'Restart', cmd.restart)
		self.addAction('menu', False,'Quit && Exit Clients', cmd.quitExitClients)
		self.addAction('menu', False,'Quit', cmd.quit)

		self.setContextMenu( self.menu['menu'])

		# Prepare Icons:
		self.icons = dict()
		icon_filename = cgruconfig.VARS['tray_icon']
		if icon_filename is None: icon_filename = 'keeper'
		icon_filename = cgruutils.getIconFileName( icon_filename)
		self.icon_epmty = QtGui.QPixmap( icon_filename)
		self.icons['empty'] = QtGui.QIcon( self.icon_epmty)
		self.makeIcon('offline_free',		 online=False, nimby=False, busy=False)
		self.makeIcon('online_free',		  online=True,  nimby=False, busy=False)
		self.makeIcon('offline_nimby',		online=False, nimby=True,  busy=False)
		self.makeIcon('online_nimby',		 online=True,  nimby=True,  busy=False)
		self.makeIcon('offline_free_busy',  online=False, nimby=False, busy=True )
		self.makeIcon('online_free_busy',	online=True,  nimby=False, busy=True )
		self.makeIcon('offline_nimby_busy', online=False, nimby=True,  busy=True )
		self.makeIcon('online_nimby_busy',  online=True,  nimby=True,  busy=True )

		# Decorate and show:
		self.showIcon()
		self.setToolTip( cgruconfig.VARS['company'].upper() + ' Keeper ' + os.getenv('CGRU_VERSION', ''))
		QtCore.QObject.connect( self, QtCore.SIGNAL('activated( QSystemTrayIcon::ActivationReason)'), self.activated_slot)

		self.show()

	def addMenu( self, parentmenu, menuname, iconname = None):
		if menuname in self.menu: return False
		if iconname is None: iconname = menuname.lower().replace(' ','_').replace('.','')
		iconpath = cgruutils.getIconFileName( iconname)
		if iconpath is not None:
			self.menu[menuname] = parentmenu.addMenu( QtGui.QIcon( iconpath), menuname)
		else:
			self.menu[menuname] = parentmenu.addMenu( menuname)
		return True

	def addAction( self, menuname, separator, actionname, function, iconname = None):
		if separator: self.menu[menuname].addSeparator()
		if iconname is None: iconname = actionname.lower().replace(' ','_').replace('.','')
		iconpath = cgruutils.getIconFileName( iconname)
		action = QtGui.QAction( actionname, self)
		if iconpath is not None: action.setIcon( QtGui.QIcon( iconpath))
		QtCore.QObject.connect( action, QtCore.SIGNAL('triggered()'), function)
		self.menu[menuname].addAction( action)				

	def makeIcon( self, name, online, nimby, busy):
		painting = self.icon_epmty
		painter = QtGui.QPainter( painting)
		icon_size = painting.width()
		text_font = QtGui.QFont('Arial',icon_size/3)
		text_font.setBold( True)
		rect_back = QtCore.QRect( icon_size*3/10, icon_size*3/10, icon_size*2/5, icon_size*2/5)
		text_color = QtGui.QColor( 0, 0, 0)
		back_color = QtGui.QColor( 150, 150, 150)
		if online:
			if nimby:
				if busy:
					text_color = QtGui.QColor( 255,	0,	0)
					back_color = QtGui.QColor(  50,  50, 250)
				else:
					text_color = QtGui.QColor( 190, 190, 190)
					back_color = QtGui.QColor(  40,  40, 240)
			else:
				if busy:
					text_color = QtGui.QColor( 255,	0,	0)
					back_color = QtGui.QColor(  90,  90,  90)
				else:
					text_color = QtGui.QColor(	0, 200,	0)
					back_color = QtGui.QColor(  90,  90,  90)
		elif nimby:
			back_color = QtGui.QColor( 140, 140, 250)
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
	def editNimby( self): self.dialog_nimby = NimbyDialog()
