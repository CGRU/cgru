import os, sys

# Write qt.conf file on MS Windows before importing PyQt:
if sys.platform.find('win') == 0:
   qtconf=sys.prefix + '/qt.conf'
   pyqt4dir=sys.prefix.replace('\\','/') + '/Lib/site-packages/PyQt4'
   if os.path.isfile( qtconf): os.remove( qtconf)
   qtconf_file = open(qtconf, mode='w')
   qtconf_file.write('[Paths]\n')
   qtconf_file.write('Prefix = ' + pyqt4dir + '\n')
   qtconf_file.write('Binaries = ' + pyqt4dir + '\n')
   qtconf_file.close()

from PyQt4 import QtGui

import cgruconfig
import cmd
from refresh import Refresh
from tray import Tray
from server import Server

# Define keeper launch command if was not:
keeper = os.getenv('CGRU_KEEPER_CMD')
if keeper is None:
   if sys.platform[:3] == 'win': keeper = 'keeper.cmd'
   else: keeper = 'keeper.sh'
   keeper = os.path.join( os.getenv('CGRU_KEEPER'), keeper)
cgruconfig.VARS['CGRU_KEEPER_CMD'] = keeper

# Set a default refresh interval in seconds:
if 'keeper_refresh' not in cgruconfig.VARS: cgruconfig.VARS['keeper_refresh'] = '36'

# Create tray application with refresh:
app = QtGui.QApplication( sys.argv)
app.setQuitOnLastWindowClosed ( False)
cmd.application = app
Tray( app)
Refresh( app)
Server( app)
app.exec_()
