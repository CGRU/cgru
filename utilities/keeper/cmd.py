# -*- coding: utf-8 -*-

import json
import subprocess

import cgrudocs
import cgruconfig
import cgruutils

import info

import af

Application = None
Tray = None
WndInfo = None

from Qt import QtCore, QtWidgets, QtCompat


def getVar(var, title='Set Variable', label='Enter new value:'):
	oldvalue = ''
	if var in cgruconfig.VARS:
		oldvalue = cgruconfig.VARS[var]

	newvalue, ok = QtWidgets.QInputDialog.getText(
		None, title, label, text=oldvalue
	)

	if not ok:
		return

	cgruconfig.VARS[var] = str(newvalue)
	variables = [var]
	cgruconfig.writeVars(variables)


def showInfo():
    global WndInfo
    WndInfo = info.Window()


def cgruDocs():
	cgrudocs.show()

def cgruForum():
	cgrudocs.showForum()


def confReload():
	cgruconfig.reconfigure()


def quit():
	Application.quit()


def setAFANASYServer():
	getVar(
		'af_servername',
		'Set AFANASY Server',
		'Enter host name or IP address:'
	)


def setDocsURL():
	getVar(
		'docshost',
		'Set Docs Host',
		'Enter host name or IP address:'
	)


def setTextEditor():
	getVar(
		'editor',
		'Set Text Editor',
		'Enter command with "%s":'
	)


def setWebBrowser():
	getVar(
		'webbrowser',
		'Set Web Browser',
		'Enter command with "%s":'
	)


def setOpenCmd():
	getVar(
		'open_folder_cmd',
		'Set Open Folder Command',
		'Enter command with "@PATH@":'
	)


def afwebgui():
	cgruutils.webbrowse(
		'%s:%s' % (
			cgruconfig.VARS['af_servername'],
			cgruconfig.VARS['af_serverport']
		)
	)


def exitRender(text='(keeper)'):
	af.Cmd().renderExit(text)


def exitMonitor(text='(keeper)'):
	af.Cmd().monitorExit(text)


def exitClients(text='(keeper)'):
	exitRender(text)
	exitMonitor(text)


def quitExitClients():
	exitClients('(keeper quit)')
	Application.quit()


def editCGRUConfig():
	cmd = cgruconfig.VARS['editor'] % cgruconfig.VARS['config_file_home']
	if QtCore.QProcess.execute(cmd) == 0:
		cgruconfig.Config()


def restart():
	QtCore.QProcess.startDetached(cgruconfig.VARS['CGRU_KEEPER_CMD'])
	Application.quit()


def update():
	exitClients('(keeper update)')
	QtCore.QProcess.startDetached(cgruconfig.VARS['CGRU_UPDATE_CMD'])
	Application.quit()


def execute( i_str):

    print('Execute:')
    print( i_str)

    obj = None

    try:
        obj = json.loads( i_str)
    except:
        return

    if not 'cmdexec' in obj:
        print('"cmdexec" object missing.')
        return

    cmdexec = obj['cmdexec']

    if 'cmds' in cmdexec:
        cmds = cmdexec['cmds']

        for cmd in cmds:
            print('Executing command:')
            print(cmd)
            subprocess.Popen( cmd, shell=True)

    if 'open' in cmdexec:
        cmd = cgruconfig.VARS['open_folder_cmd'].replace('@PATH@',cmdexec['open'])
        print('Opening folder:')
        print(cmd)
        subprocess.Popen( cmd, shell=True)

    if 'eval' in cmdexec:
        cmd = cmdexec['eval']
        print('Evaluating code:')
        print(cmd)
        eval(cmd)

