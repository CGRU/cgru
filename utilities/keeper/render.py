# -*- coding: utf-8 -*-

import json

import af
import cmd
import cgruconfig

WndInfo = None

from Qt import QtCore, QtGui, QtWidgets

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
        cmd.Tray.showIcon( makeIcon( renders[0]))
    else:
        cmd.Tray.showIcon()

def makeIcon( i_render):

    if not 'state' in i_render:
        cmd.Tray.showIcon()
        return

    online = i_render['state'].find('OFF') == -1
    busy   = i_render['state'].find('RUN') != -1
    NIMBY  = i_render['state'].find('NBY') != -1
    nimby  = i_render['state'].find('NbY') != -1 or NIMBY
    paused = i_render['state'].find('PAU') != -1

    pixmap = cmd.Tray.getIconPixmap()
    painter = QtGui.QPainter(pixmap)
    icon_size = pixmap.width()
    text_font = QtGui.QFont('Arial', icon_size / 3)
    text_font.setBold(True)
    rect_back = QtCore.QRect(
        icon_size * 3 / 10,
        icon_size * 3 / 10,
        icon_size * 2 / 5,
        icon_size * 2 / 5
    )
    text_color = QtGui.QColor(0, 0, 0)
    back_color = QtGui.QColor(150, 150, 150)
    if online:
        if paused:
            if busy:
                text_color = QtGui.QColor(255, 0, 0)
                back_color = QtGui.QColor(50, 50, 50)
            else:
                text_color = QtGui.QColor(190, 190, 190)
                back_color = QtGui.QColor(40, 40, 40)
        elif nimby:
            if busy:
                text_color = QtGui.QColor(255, 0, 0)
                back_color = QtGui.QColor(50, 50, 250)
            else:
                text_color = QtGui.QColor(190, 190, 190)
                back_color = QtGui.QColor(40, 40, 240)
        else:
            if busy:
                text_color = QtGui.QColor(255, 0, 0)
                back_color = QtGui.QColor(90, 90, 90)
            else:
                text_color = QtGui.QColor(0, 200, 0)
                back_color = QtGui.QColor(90, 90, 90)
    elif paused:
        back_color = QtGui.QColor(140, 140, 140)
    elif nimby:
        back_color = QtGui.QColor(140, 140, 250)

    rect_render = QtCore.QRect(
        icon_size / 4,
        icon_size / 4,
        icon_size / 2,
        icon_size / 2
    )
    painter.fillRect(rect_back, back_color)
    painter.setFont(text_font)
    painter.setPen(text_color)
    painter.drawText(rect_render, QtCore.Qt.AlignCenter, 'R')
    return QtGui.QIcon(pixmap)

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

