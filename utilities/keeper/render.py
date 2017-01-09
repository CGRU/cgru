# -*- coding: utf-8 -*-

import json

import af
import cmd
import cgruconfig

WndInfo = None

from Qt import QtCore, QtWidgets

def showInfo(tray=None):
	renders = af.Cmd().renderGetLocal()

	if renders is None or len(renders) == 0:
		if tray is not None:
			tray.showMessage(
				'Render information:',
				'No local render client found.'
			)
		return

	global WndInfo

	WndInfo = QtWidgets.QTextEdit()
	WndInfo.setPlainText(json.dumps(renders[0], sort_keys=True, indent=4))
	WndInfo.setReadOnly(True)
	WndInfo.resize(WndInfo.viewport().size())
	WndInfo.setWindowTitle('AFANASY Render Information:')
	WndInfo.show()


def refresh():
	renders = af.Cmd().renderGetLocal()
	if renders is not None and len(renders):
		render = renders[0]
		if "state" in render:
			online = render["state"].find('OFF') == -1
			nimby = render["state"].find('NbY') != -1
			NIMBY = render["state"].find('NBY') != -1
			busy = render["state"].find('RUN') != -1
			cmd.Tray.showRenderIcon(online, nimby or NIMBY, busy)
			cmd.Tray.showUser(render["user_name"])
	else:
		cmd.Tray.showIcon()

def setUserDialog():
	name, ok = QtWidgets.QInputDialog.getText(
		None, 'Set Render User', 'Ender New Name:', text=cgruconfig.VARS['USERNAME']
	)

	if not ok:
		return

	af.Cmd().renderSetUserName( name)
	refreshAfter()

def refreshAfter( i_sec = 3):
	timer = QtCore.QTimer( cmd.Application)
	timer.timeout.connect( refresh)
	timer.setInterval( 1000 * i_sec)
	timer.setSingleShot( True)
	timer.start()

