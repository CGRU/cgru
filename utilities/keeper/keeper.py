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
	af_root = os.getenv('AF_ROOT')
	if af_root:
		qtconfs.append(os.path.join(af_root, 'bin', 'qt.conf'))
	cgru_loc = os.getenv('CGRU_LOCATION')
	# Prefer CGRU/dll from the Windows release package; fall back to PySide2
	# (git/source clones often do not include dll/ or afanasy/bin).
	qtdll = None
	if cgru_loc and os.path.isdir(os.path.join(cgru_loc, 'dll')):
		qtdll = cgru_loc.replace('\\', '/') + '/dll'
	else:
		pyside = os.path.join(sys.prefix, 'Lib', 'site-packages', 'PySide2')
		if not os.path.isdir(pyside):
			pyside = os.path.join(sys.prefix, 'Lib', 'site-packages', 'PySide6')
		if os.path.isdir(pyside):
			qtdll = pyside.replace('\\', '/')
			print('WARNING: CGRU dll/ not found, using Qt from: ' + qtdll)
		elif cgru_loc:
			qtdll = cgru_loc.replace('\\', '/') + '/dll'
		else:
			qtdll = 'dll'
	qtconf_txt = '[Paths]\nPrefix = ' + qtdll + '\n'
	for qtconf in qtconfs:
		qtconf_dir = os.path.dirname(qtconf)
		if qtconf_dir and not os.path.isdir(qtconf_dir):
			try:
				os.makedirs(qtconf_dir)
			except Exception:
				continue
		try:
			if os.path.isfile(qtconf):
				os.remove(qtconf)
			qtconf_file = open(qtconf, mode='w')
			qtconf_file.write(qtconf_txt)
			qtconf_file.close()
		except Exception as err:
			print('WARNING: Can`t write "%s": %s' % (qtconf, err))

if sys.platform.find('darwin') == 0:
    try:
        import AppKit
        info = AppKit.NSBundle.mainBundle().infoDictionary()
        info["LSBackgroundOnly"] = "1"
    except:
        print("WARNING: pyobjc-framework-Cocoa is not installed, can't hide dock icon.")

import Qt
from Qt import QtWidgets

print('Qt: ' + ('%s %s (%s)' % ( Qt.__qt_version__, Qt.__binding__, Qt.__binding_version__)))

import cgruconfig
import cmd
from refresh import Refresh
import render
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
render.Refresh = refresh
server = Server()
# PySide6: exec(); PySide2: exec_()
getattr(app, 'exec', getattr(app, 'exec_', None))()
