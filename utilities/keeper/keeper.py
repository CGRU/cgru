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

from cgrupyqt import QtGui

import cgruconfig
import cmd
from refresh import Refresh
from tray import Tray
from server import Server

# Define keeper launch command if was not:
keeper_cmd = os.getenv('CGRU_KEEPER_CMD')
if keeper_cmd is None:
   keeper_cmd = '"%s" "%s"' % ( os.getenv('CGRU_PYTHONEXE'), sys.argv[0])
cgruconfig.VARS['CGRU_KEEPER_CMD'] = keeper_cmd

# Create tray application with refresh:
app = QtGui.QApplication( sys.argv)
app.setQuitOnLastWindowClosed ( False)
cmd.Application = app
cmd.Tray = Tray( app)
refresh = Refresh( app)
server = Server( app)
app.exec_()
