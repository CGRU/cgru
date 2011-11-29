import af

WndInfo = None

from PyQt4 import QtGui

def showInfo( tray = None):
   renders = af.Cmd().renderGetLocal()

   if len( renders) == 0:
      if tray is not None: tray.showMessage('Render information:', 'No local render client founded.')
      return

   global WndInfo

   WndInfo = QtGui.QTextEdit()
   msg = ''
   for rinfo in renders:
      if isinstance(rinfo['info'], str):
         msg += rinfo['info']
         msg += rinfo['resources']
      else:
         msg += str(rinfo['info'], 'utf-8')
         msg += str(rinfo['resources'], 'utf-8')
   WndInfo.setPlainText( msg)
   WndInfo.setReadOnly( True)
   WndInfo.resize( WndInfo.viewport().size())
   WndInfo.setWindowTitle('AFANASY Render Information:')
   WndInfo.show()
   
def refresh():
   renders = af.Cmd().renderGetLocal()
   print( renders)
