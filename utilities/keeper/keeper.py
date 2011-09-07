import os, sys

# Write qt.conf file on MS Windows before importing PyQt:
if sys.platform.find('win') == 0:
   qtconf=os.getenv('PYTHON') + '/qt.conf'
   pyqt4dir=os.getenv('PYTHON').replace('\\','/') + '/Lib/site-packages/PyQt4'
   if os.path.isfile( qtconf): os.remove( qtconf)
   qtconf_file = open(qtconf, mode='w')
   qtconf_file.write('[Paths]\n')
   qtconf_file.write('Prefix = ' + pyqt4dir + '\n')
   qtconf_file.write('Binaries = ' + pyqt4dir + '\n')
   qtconf_file.close()

from PyQt4 import QtCore, QtGui

import cgruconfig
import refresh
from tray import Tray

refresh.refresh()

# Check for a text editor, it always must be defined:
if 'editor' not in cgruconfig.VARS:
   if sys.platform.find('win') == 0:
      cgruconfig.VARS['editor'] = 'notepad "%s"'
   else:
      cgruconfig.VARS['editor'] = 'xterm -e vi "%s"'

app = QtGui.QApplication( sys.argv)
app.setQuitOnLastWindowClosed ( False)
tray = Tray( app)
timer = QtCore.QTimer( app)
timer.setInterval(10000)
QtCore.QObject.connect( timer, QtCore.SIGNAL('timeout()'), refresh.refresh)
timer.start()

app.exec_()
