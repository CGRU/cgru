import os, sys

# Write qt.conf file on MS Windows before importing PyQt:
if sys.platform.find('win') == 0:
   qtconf=os.getenv('PYTHON') + '/qt.conf'
   pyqt4dir=os.getenv('PYTHON').replace('\\','/') + '/Lib/site-packages/PyQt4'
   if os.path.isfile( qtconf): os.remove( qtconf)
   qtconf_file = open(qtconf, mode='w')
   print('[Paths]', file=qtconf_file)
   print('Prefix = ' + pyqt4dir, file=qtconf_file)
   print('Binaries = ' + pyqt4dir, file=qtconf_file)
   qtconf_file.close()

from PyQt4 import QtCore, QtGui

import cgruconfig
from tray import Tray

# Check for a text editor, it always must be defined:
if 'editor' not in cgruconfig.VARS:
   if sys.platform.find('win') == 0:
      cgruconfig.VARS['editor'] = 'notepad "%s"'
   else:
      cgruconfig.VARS['editor'] = 'xterm -e vi "%s"'

app = QtGui.QApplication( sys.argv)
app.setQuitOnLastWindowClosed ( False)
icon = QtGui.QIcon( os.path.join( os.path.join( os.getenv('CGRU_KEEPER', ''), 'icons'), 'keeper.svg'))
app.setWindowIcon( icon)
tray = Tray( app)

app.exec_()
