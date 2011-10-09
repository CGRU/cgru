import os, sys

import cgruconfig
import render

application = None

from PyQt4 import QtCore

def editCGRUConfig():
   if QtCore.QProcess.execute( cgruconfig.VARS['editor'] % cgruconfig.VARS['HOME_CONFIGFILE']) == 0:
      cgruconfig.Config()

def startjob():
   cmd = os.path.join('afstarter','afstarter.py')
   cmd = os.path.join('utilities', cmd)
   cmd = os.path.join( cgruconfig.VARS['CGRU_LOCATION'], cmd)
   QtCore.QProcess.startDetached( cgruconfig.VARS['CGRU_PYTHONEXE'], [cmd])

def checkregexp():
   cmd = os.path.join( cgruconfig.VARS['CGRU_LOCATION'], 'utilities')
   cmd = os.path.join( os.path.join( cmd, 'regexp'), 'regexp.py')
   QtCore.QProcess.startDetached( cgruconfig.VARS['CGRU_PYTHONEXE'], [cmd])

def restart():
   QtCore.QProcess.startDetached( cgruconfig.VARS['CGRU_KEEPER_CMD'])
   application.quit()

def quit(): application.quit()

def update():
   render.exit()
   render.exitmonitor()
   render.exittalk()
   QtCore.QProcess.startDetached( cgruconfig.VARS['CGRU_UPDATE_CMD'])
   application.quit()
