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

# Set a default keeper trat icon if not defined:
if 'tray_icon' not in cgruconfig.VARS: cgruconfig.VARS['tray_icon'] = None

# Check for a text editor, it always must be defined:
if 'editor' not in cgruconfig.VARS:
   if sys.platform.find('win') == 0:
      cgruconfig.VARS['editor'] = 'notepad "%s"'
   else:
      cgruconfig.VARS['editor'] = 'xterm -e vi "%s"'

# Define keeper launch command if was not:
keeper = os.getenv('CGRU_KEEPER_CMD')
if keeper is None:
   if sys.platform[:3] == 'win': keeper = 'keeper.cmd'
   else: keeper = 'keeper.sh'
   keeper = os.path.join( os.getenv('CGRU_KEEPER'), keeper)
cgruconfig.VARS['CGRU_KEEPER_CMD'] = keeper

# Include CGRU_UPDATE_CMD:
cgruconfig.VARS['CGRU_UPDATE_CMD'] = os.getenv('CGRU_UPDATE_CMD')

app = QtGui.QApplication( sys.argv)
app.setQuitOnLastWindowClosed ( False)
tray = Tray( app)
timer = QtCore.QTimer( app)
timer.setInterval(10000)
QtCore.QObject.connect( timer, QtCore.SIGNAL('timeout()'), refresh.refresh)
timer.start()

app.exec_()
