import af

import cmd

WndInfo = None

from PyQt4 import QtCore, QtGui

def showInfo( tray = None):
   renders = af.Cmd().renderGetLocal()

   if len( renders) == 0:
      if tray is not None: tray.showMessage('Render information:', 'No local render client founded.')
      return

   global WndInfo

   WndInfo = QtGui.QTextEdit()
   msg = ''
   isstring = False
   if isinstance( renders[0]['info'], str): isstring = True
   for rinfo in renders:
      if isstring:
         msg += rinfo['info']
         msg += rinfo['resources']
      else:
         msg += str(rinfo['info'], 'utf-8')
         msg += str(rinfo['resources'], 'utf-8')
   if isstring: msg = QtCore.QString.fromUtf8( msg)
   WndInfo.setPlainText( msg)
   WndInfo.setReadOnly( True)
   WndInfo.resize( WndInfo.viewport().size())
   WndInfo.setWindowTitle('AFANASY Render Information:')
   WndInfo.show()
   
def refresh():
   renders = af.Cmd().renderGetLocal()
   if renders is not None and len( renders):
      render = renders[0]
      #print( render)
      cmd.Tray.showRenderIcon( render['online'], render['nimby'] or render['NIMBY'], render['busy'])
   else: cmd.Tray.showIcon()
