import os, sys

import cgrudocs
import cgruconfig
import cgruutils

import af

Application = None
Tray = None

from cgrupyqt import QtCore, QtGui

def getVar( var, title = 'Set Variable', label = 'Enter new value:'):
   oldvalue = ''
   if var in cgruconfig.VARS: oldvalue = cgruconfig.VARS[var]
   newvalue, ok = QtGui.QInputDialog.getText( None, title, label, text = oldvalue)
   if not ok: return
   cgruconfig.VARS[var] = str( newvalue)
   variables = [var]
   cgruconfig.writeVars(variables)

def cgruDocs(): cgrudocs.show()
def confReload(): cgruconfig.Config()
def quit(): Application.quit()
def setAFANASYServer(): getVar('af_servername','Set AFANASY Server','Enter host name or IP address:')
def setDocsURL(): getVar('docshost','Set Docs Host','Enter host name or IP address:')
def setTextEditor(): getVar('editor','Set Text Editor','Enter command with "%s":')
def setWebBrowser(): getVar('webbrowser','Set Web Browser','Enter command with "%s":')
def afwebgui(): cgruutils.webbrowse( cgruconfig.VARS['af_servername']+':'+str(cgruconfig.VARS['af_serverport']))

def exitRender(  text = '(keeper)'): af.Cmd().renderExit( text)
def exitTalk(    text = '(keeper)'): af.Cmd().talkExit( text)
def exitMonitor( text = '(keeper)'): af.Cmd().monitorExit( text)
def exitClients( text = '(keeper)'):
   exitRender( text)
   exitTalk( text)
   exitMonitor( text)

def quitExitClients():
   exitClients('(keeper quit)')
   Application.quit()

def editCGRUConfig():
   if QtCore.QProcess.execute( cgruconfig.VARS['editor'] % cgruconfig.VARS['config_file_home']) == 0:
      cgruconfig.Config()

def restart():
   QtCore.QProcess.startDetached( cgruconfig.VARS['CGRU_KEEPER_CMD'])
   Application.quit()

def update():
   exitClients('(keeper update)')
   QtCore.QProcess.startDetached( cgruconfig.VARS['CGRU_UPDATE_CMD'])
   Application.quit()
