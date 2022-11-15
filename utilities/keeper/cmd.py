# -*- coding: utf-8 -*-

import json
import os
import subprocess
import sys

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


def setTerminal():
	getVar(
		'open_terminal_cmd',
		'Set Open Terminal Command',
		'Enter command with @CMD@:'
	)


def afwebgui():
	cgruutils.webbrowse(
		'%s:%s' % (
			cgruconfig.VARS['af_servername'],
			cgruconfig.VARS['af_serverport']
		)
	)


def exitRender():
	af.Cmd().renderExit()


def exitMonitor():
	af.Cmd().monitorExit()


def exitClients():
	exitRender()
	exitMonitor()


def quitExitClients():
	exitClients()
	Application.quit()


def editCGRUConfig():
	cmd = cgruconfig.VARS['editor'] % cgruconfig.VARS['config_file_home']
	if QtCore.QProcess.execute(cmd) == 0:
		cgruconfig.Config()


def restart():
	QtCore.QProcess.startDetached(cgruconfig.VARS['CGRU_KEEPER_CMD'])
	Application.quit()


def update():
	exitClients()
	QtCore.QProcess.startDetached(cgruconfig.VARS['CGRU_UPDATE_CMD'])
	Application.quit()


def triggerExecInTerminal(i_checked):
    var = 'keeper_execute_in_terminal'
    cgruconfig.VARS[var] = i_checked
    variables = [var]
    cgruconfig.writeVars(variables)


def execute( i_str):

    print('Execute:')
    print( i_str)

    obj = None

    try:
        obj = json.loads(i_str)
    except:
        return False, 'Bad JSON object received.'

    if not 'cmdexec' in obj:
        error = '"cmdexec" object missing.'
        print(error)
        return False, error

    cmdexec = obj['cmdexec']
    cmd = None

    if 'cmds' in cmdexec:
        cmds = cmdexec['cmds']

        for cmd in cmds:
            print('Executing command:')
            print(cmd)
            subprocess.Popen(cmd, shell=True)

        return True, None

    if 'cmd' in cmdexec:
        cmd = cmdexec['cmd']
        if cgruconfig.getVar('keeper_execute_in_terminal'):
            if sys.platform.find('win') == 0:
                cmd = 'start cmd.exe /C "%s"' % cmd
            else:
                cmd = cgruconfig.VARS['open_terminal_cmd'].replace('@CMD@', cmd)
        print('Executing command:')

    elif 'open' in cmdexec:
        folder = cmdexec['open']
        cmd = cgruconfig.VARS['open_folder_cmd'].replace('@PATH@', folder)
        if not os.path.isdir(folder):
            return False, ('Folder\n"%s"\ndoes not exist.' % folder)
        print('Opening folder:')

    elif 'terminal' in cmdexec:
        if sys.platform.find('win') == 0:
            cmd = ('start cmd.exe /K "cd %s"' % cmdexec['terminal'])
        else:
            cmd = ('cd %s; openterminal' % cmdexec['terminal'])
        print('Opening terminal:')

    elif 'eval' in cmdexec:
        cmd = cmdexec['eval']
        print('Evaluating code:')
        print(cmd)
        eval(cmd)
        return True, None

    else:
        return False, 'Invalid request object. Nothing to do.'

    if cmd is None:
        return False, 'Invalid request object. No command to execute.'

    print(cmd)
    p = subprocess.Popen(cmd, shell=True, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    status = None
    stderr = None
    stdout = None
    try:
        status = p.wait(2)
        stdout, stderr = p.communicate()
    except:
        pass

    output = ''
    if stdout is not None:
        output += stdout.decode('utf-8')
    if stderr is not None:
        if len(output):
            output += '\n\n'
        output += stderr.decode('utf-8')
    if len(output) == 0:
        output = None

    if status is not None and status != 0:
        return False, output

    return True, output

