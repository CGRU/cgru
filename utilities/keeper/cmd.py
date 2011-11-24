import os, sys

import cgrudocs
import cgruconfig
import render

application = None

from PyQt4 import QtCore, QtGui

def getVar( var, title = 'Set Variable', label = 'Enter new value:'):
   oldvalue = ''
   if var in cgruconfig.VARS: oldvalue = cgruconfig.VARS[var]
   newvalue, ok = QtGui.QInputDialog.getText( None, title, label, text = oldvalue)
   if not ok: return
   cgruconfig.VARS[var] = str( newvalue)
   variables = [var]
   cgruconfig.writeVars(variables)

def getAfVar( var, title = 'Set Variable', label = 'Enter new value:'):
   oldvalue = ''
   if var in cgruconfig.VARS: oldvalue = cgruconfig.VARS[var]
   newvalue, ok = QtGui.QInputDialog.getText( None, title, label, text = oldvalue)
   if not ok: return
   cgruconfig.VARS[var] = str( newvalue)
   variables = [var]
   cgruconfig.writeVars(variables, cgruconfig.VARS['config_afanasy'])

def cgruDocs(): cgrudocs.show()
def confReload(): cgruconfig.Config()
def quit(): application.quit()
def setAFANASYServer(): getAfVar('servername','Set AFANASY Server','Enter host name or IP address:')
def setDocsURL(): getVar('docshost','Set Docs Host','Enter host name or IP address:')
def setTextEditor(): getVar('editor','Set Text Editor','Enter command with "%s":')

def editCGRUConfig():
   if QtCore.QProcess.execute( cgruconfig.VARS['editor'] % cgruconfig.VARS['config_file_home']) == 0:
      cgruconfig.Config()

def editAFANASYConfig():
   if QtCore.QProcess.execute( cgruconfig.VARS['editor'] % cgruconfig.VARS['config_afanasy']) == 0:
      cgruconfig.Config()

def restart():
   QtCore.QProcess.startDetached( cgruconfig.VARS['CGRU_KEEPER_CMD'])
   application.quit()

def update():
   render.exit()
   render.exitmonitor()
   render.exittalk()
   QtCore.QProcess.startDetached( cgruconfig.VARS['CGRU_UPDATE_CMD'])
   application.quit()
