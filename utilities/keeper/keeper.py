# -*- coding: utf-8 -*-

import sys
import os

if sys.platform.find('win') == 0 and sys.executable.find('pythonw') != -1:
	try:
		sys.stdout = open('c:\\temp\\cgru_keeper_log.txt', 'w')
		sys.stderr = sys.stdout
	except:
		pass

# Write qt.conf file on MS Windows:
if sys.platform.find('win') == 0:
	qtconfs = []
	qtconfs.append(sys.prefix + '/qt.conf')
	qtconfs.append(os.getenv('AF_ROOT') + '/bin/qt.conf')
	qtdll = os.getenv('CGRU_LOCATION').replace('\\', '/') + '/dll'
	qtconf_txt = '[Paths]\nPrefix = ' + qtdll + '\n'
	for qtconf in qtconfs:
		if os.path.isfile(qtconf):
			os.remove(qtconf)
		qtconf_file = open(qtconf, mode='w')
		qtconf_file.write(qtconf_txt)
		qtconf_file.close()

if sys.platform.find('darwin') == 0:
    try:
        import AppKit
        info = AppKit.NSBundle.mainBundle().infoDictionary()
        info["LSBackgroundOnly"] = "1"
    except:
        print("WARNING: pyobjc-framework-Cocoa is not installed, can't hide dock icon.")


from Qt import QtWidgets

import cgruconfig
import cmd
from refresh import Refresh
from tray import Tray
from server import Server
import serverhttps


# Define keeper launch command if was not:
keeper_cmd = os.getenv('CGRU_KEEPER_CMD')
if keeper_cmd is None:
	keeper_cmd = '"%s" "%s"' % (os.getenv('CGRU_PYTHONEXE'), sys.argv[0])
cgruconfig.VARS['CGRU_KEEPER_CMD'] = keeper_cmd

# Create tray application with refresh:
app = QtWidgets.QApplication(sys.argv)
app.setQuitOnLastWindowClosed(False)
cmd.Application = app
serverhttps.serve(cgruconfig.VARS['keeper_port_https'])
cmd.Tray = Tray(app)
refresh = Refresh(app)
server = Server()
app.exec_()
