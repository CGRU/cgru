# -*- coding: utf-8 -*-
import os
import sys
import subprocess

import cgruconfig
import cgruutils

import af

import cmd
import info
import nimby
import render
import serverhttps
import software

from nimby_dialog import NimbyDialog

from Qt import QtCore, QtGui, QtWidgets


class ActionCommand(QtWidgets.QAction):
    def __init__(self, parent, name, command, iconpath=None):
        QtWidgets.QAction.__init__(self, name, parent)
        if iconpath is not None:
            iconpath = cgruutils.getIconFileName(iconpath)
            if iconpath is not None:
                self.setIcon(QtGui.QIcon(iconpath))
            else:
                print('WARNING: Icon "%s" does not exist.' % iconpath)
        self.name = name
        self.cmd = command

    def runCommand(self):
        QtCore.QProcess.startDetached(self.cmd, [])


class Tray(QtWidgets.QSystemTrayIcon):
    def __init__(self, parent=None):
        QtWidgets.QSystemTrayIcon.__init__(self, parent)
        self.parent = parent

        # Menu:
        self.menu = dict()
        self.menu['menu'] = QtWidgets.QMenu()

        # Update item only if CGRU_UPDATE_CMD defined:
        if cgruconfig.VARS['CGRU_UPDATE_CMD'] is not None:
            action = QtWidgets.QAction('Update', self)
            action.triggered.connect( cmd.update)
            self.menu['menu'].addAction(action)
            self.menu['menu'].addSeparator()

        # Load menu:
        menu_path = os.path.join(
            os.path.join(cgruconfig.VARS['CGRU_LOCATION'], 'start')
        )  # TODO: why to use os.path.join() two times here, to add '/' at the end ???
        menu_paths = cgruconfig.VARS['menu_path']
        if menu_paths is None:
            menu_paths = menu_path
        if menu_paths.find(';') != -1:
            menu_paths = menu_paths.split(';')
        elif sys.platform.find('win') == -1:
            menu_paths = menu_paths.split(':')
        else:
            menu_paths = [menu_paths]
        if not menu_path in menu_paths:
            menu_paths.append(menu_path)
        for menu_path in menu_paths:
            if not os.path.isdir(menu_path):
                continue
            for dirpath, dirnames, filenames in os.walk(menu_path, True, None,
                                                        True):
                if dirpath.find('/.') != -1:
                    continue
                if dirpath.find('\\.') != -1:
                    continue
                menuname = os.path.basename(dirpath)
                if menuname == os.path.basename(menu_path):
                    menuname = 'menu'
                else:
                    self.addMenu(self.menu['menu'], menuname)
                filenames.sort()
                was_separator = True
                for filename in filenames:
                    if filename[0] == '.' or filename[0] == '_':
                        continue
                    if sys.platform[:3] == 'win':
                        if filename[-4:] != '.cmd':
                            continue
                        itemname = filename[:-4]
                    else:
                        if filename[-3:] != '.sh':
                            continue
                        itemname = filename[:-3]
                    filename = os.path.join(dirpath, filename)
                    with open(filename, 'r') as f:
                        lines = f.readlines()

                    iconpath = None
                    separator = False
                    for line in lines:
                        if line.find('Name=') != -1:
                            itemname = line.split('Name=')[-1].strip()
                        if line.find('Icon=') != -1:
                            iconpath = line.split('Icon=')[-1].strip()
                        if line.find('Separator') != -1:
                            separator = True
                    if separator:
                        if not was_separator:
                            self.menu[menuname].addSeparator()
                        was_separator = True
                    else:
                        was_separator = False
                    action = ActionCommand(self, itemname, filename, iconpath)
                    self.menu[menuname].addAction(action)
                    action.triggered.connect( action.runCommand)
                    if separator:
                        self.menu[menuname].addSeparator()

        # Add permanent items to 'Afanasy':
        if not self.addMenu(self.menu['menu'], 'AFANASY'):
            self.menu['AFANASY'].addSeparator()

        action = QtWidgets.QAction('Web GUI', self)
        action.triggered.connect( cmd.afwebgui)
        self.menu['AFANASY'].addAction(action)
        self.menu['AFANASY'].addSeparator()

        self.action_user = QtWidgets.QAction('Set User...', self)
        self.action_user.triggered.connect( render.setUserDialog)
        self.action_user.setToolTip('Change local render user name.')
        self.menu['AFANASY'].addAction(self.action_user)

        action = QtWidgets.QAction('Set nimby', self)
        action.triggered.connect( nimby.setnimby)
        self.menu['AFANASY'].addAction(action)

        action = QtWidgets.QAction('Set NIMBY', self)
        action.triggered.connect( nimby.setNIMBY)
        self.menu['AFANASY'].addAction(action)

        action = QtWidgets.QAction('Set Free && Unpause', self)
        action.triggered.connect( nimby.setFreeUnpause)
        self.menu['AFANASY'].addAction(action)

        action = QtWidgets.QAction('Eject Tasks', self)
        action.triggered.connect( nimby.ejectTasks)
        self.menu['AFANASY'].addAction(action)

        action = QtWidgets.QAction('Eject Not My Tasks', self)
        action.triggered.connect( nimby.ejectNotMyTasks)
        self.menu['AFANASY'].addAction(action)

        action = QtWidgets.QAction('Render Info', self)
        action.triggered.connect( self.renderInfo)
        self.menu['AFANASY'].addAction(action)

        self.menu['AFANASY'].addSeparator()

        action = QtWidgets.QAction('Nimby Schedule...', self)
        action.triggered.connect( self.editNimby)
        self.menu['AFANASY'].addAction(action)

        self.menu['AFANASY'].addSeparator()

        action = QtWidgets.QAction('Set Server...', self)
        action.triggered.connect( cmd.setAFANASYServer)
        self.menu['AFANASY'].addAction(action)


        self.menu['menu'].addSeparator()

        # Add Software menu if it was not created by custom files:
        if not 'Software' in self.menu:
            self.addMenu(self.menu['menu'], 'Software')
            self.menu['menu'].addSeparator()
            action = QtWidgets.QAction(
                QtGui.QIcon(cgruutils.getIconFileName('folder')),
                '[ browse ]',
                self
            )
            action.triggered.connect( software.browse)
            self.menu['Software'].addAction(action)
            for soft in software.Names:
                icon = software.getIcon(soft)
                if icon is None:
                    action = QtWidgets.QAction(soft, self)
                else:
                    action = QtWidgets.QAction(icon, soft, self)
                eval("action.triggered.connect(software.start%s)" % soft)
                self.menu['Software'].addAction(action)
            # Software setup:
            self.menu['Setup Soft'] = QtWidgets.QMenu('Setup Soft')
            self.menu['Software'].addMenu(self.menu['Setup Soft'])
            for soft in software.Names:
                action = QtWidgets.QAction(soft, self)
                eval("action.triggered.connect(software.locate%s)" % soft)
                self.menu['Setup Soft'].addAction(action)
            # Software examples:
            self.menu['Examples'] = QtWidgets.QMenu('Examples')
            self.menu['Software'].addMenu(self.menu['Examples'])
            for soft in software.Names:
                action = QtWidgets.QAction(soft, self)
                eval("action.triggered.connect(software.example%s)" % soft)
                self.menu['Examples'].addAction(action)

        # Add permanent items to 'Configure':
        if not self.addMenu(self.menu['menu'], 'Configure'):
            self.menu['Configure'].addSeparator()

        if serverhttps.isRunning:
            self.addAction('Configure', False,  'HTTPS Server...', self.httpsServer)
            self.menu['Configure'].addSeparator()

        action = QtWidgets.QAction('Set Web Browser...', self)
        action.triggered.connect( cmd.setWebBrowser)
        self.menu['Configure'].addAction(action)

        action = QtWidgets.QAction('Set Open Folder...', self)
        action.triggered.connect( cmd.setOpenCmd)
        self.menu['Configure'].addAction(action)

        action = QtWidgets.QAction('Set Docs URL...', self)
        action.triggered.connect( cmd.setDocsURL)
        self.menu['Configure'].addAction(action)

        action = QtWidgets.QAction('Set Text Editor...', self)
        action.triggered.connect( cmd.setTextEditor)
        self.menu['Configure'].addAction(action)

        action = QtWidgets.QAction('Edit Config...', self)
        action.triggered.connect( cmd.editCGRUConfig)
        self.menu['Configure'].addAction(action)

        self.menu['Configure'].addSeparator()

        action = QtWidgets.QAction('Reload Config', self)
        action.triggered.connect( cmd.confReload)
        self.menu['Configure'].addAction(action)

        self.addAction('menu', True,  'Show Info...',         self.cgruInfo, 'info')
        self.addAction('menu', True,  'Documentation...',     cmd.cgruDocs)
        self.addAction('menu', False, 'Forum...',             cmd.cgruForum)
        self.addAction('menu', True,  'Restart',              cmd.restart)
        self.addAction('menu', False, 'Quit',                 cmd.quit)

        self.setContextMenu(self.menu['menu'])

        # Tray tooltip:
        self.setToolTip('%s Keeper' % cgruconfig.VARS['company'].upper())

        # Tray icon:
        icon_filename = cgruconfig.VARS['tray_icon']
        if icon_filename is None: icon_filename = 'keeper'
        icon_filename = cgruutils.getIconFileName(icon_filename)
        self.icon_pixmap = QtGui.QPixmap(icon_filename)
        self.icon_default = QtGui.QIcon(self.icon_pixmap)
        self.showIcon()

        # Show:
        self.activated.connect( self.activated_slot)
        self.show()

    def getIconPixmap( self): return self.icon_pixmap

    def addMenu(self, parentmenu, menuname, iconname=None):
        if menuname in self.menu:
            return False
        if iconname is None:
            iconname = menuname.lower().replace(' ', '_').replace('.', '')
        iconpath = cgruutils.getIconFileName(iconname)
        if iconpath is not None:
            self.menu[menuname] = \
                parentmenu.addMenu(QtGui.QIcon(iconpath), menuname)
        else:
            self.menu[menuname] = parentmenu.addMenu(menuname)
        return True

    def addAction(self, menuname, separator, actionname, function, iconname=None):
        if separator:
            self.menu[menuname].addSeparator()

        if iconname is None:
            iconname = actionname.lower().replace(' ', '_').replace('.', '')

        iconpath = cgruutils.getIconFileName(iconname)
        action = QtWidgets.QAction(actionname, self)

        if iconpath is not None:
            action.setIcon(QtGui.QIcon(iconpath))

        action.triggered.connect( function)
        self.menu[menuname].addAction(action)

    def showUser(self, i_user_name):
        self.action_user.setText('User: "%s"' % i_user_name)

    def showIcon(self, i_icon = None):
        if i_icon is None:
            self.icon = self.icon_default
        else:
            self.icon = i_icon
        self.setIcon( self.icon)
        self.parent.setWindowIcon( self.icon)

    def activated_slot(self, reason):
        if reason == QtWidgets.QSystemTrayIcon.Trigger:
            return
        elif reason == QtWidgets.QSystemTrayIcon.DoubleClick:
            render.refresh()
        elif reason == QtWidgets.QSystemTrayIcon.MiddleClick:
            return
        elif reason == QtWidgets.QSystemTrayIcon.Context:
            return
        elif reason == QtWidgets.QSystemTrayIcon.Unknown:
            return

    def renderInfo(self):
        render.showInfo(self)

    def cgruInfo(self):
        self.dialog_info = info.Window()

    def httpsServer(self):
        cgruutils.webbrowse('https://localhost:' + str(cgruconfig.VARS['keeper_port_https']) + '/')

    def editNimby(self):
        self.dialog_nimby = NimbyDialog()
