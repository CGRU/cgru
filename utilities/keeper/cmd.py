# -*- coding: utf-8 -*-

import cgrudocs
import cgruconfig
import cgruutils

import af

Application = None
Tray = None

from cgrupyqt import QtCore, QtGui


def getVar(var, title='Set Variable', label='Enter new value:'):
    oldvalue = ''
    if var in cgruconfig.VARS:
        oldvalue = cgruconfig.VARS[var]

    newvalue, ok = QtGui.QInputDialog.getText(
        None, title, label, text=oldvalue
    )

    if not ok:
        return

    cgruconfig.VARS[var] = str(newvalue)
    variables = [var]
    cgruconfig.writeVars(variables)


def cgruDocs():
    cgrudocs.show()


def confReload():
    cgruconfig.Config()


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
