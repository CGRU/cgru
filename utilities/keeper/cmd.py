import os, sys

import cgruconfig
import render

application = None

from PyQt4 import QtCore

def editCGRUConfig():
   if QtCore.QProcess.execute( cgruconfig.VARS['editor'] % cgruconfig.VARS['config_file_home']) == 0:
      cgruconfig.Config()

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
