# -*- coding: utf-8 -*-

import json
import time

import af
import cmd
import cgruconfig

WndInfo = None

from Qt import QtCore, QtGui, QtWidgets

Render = None
RenderFull = None
Refresh = None

def showInfo(tray=None):
    render = RenderFull
    if render is None: render = Render

    if render is None:
        if tray is not None:
            tray.showMessage(
                'Render information:',
                'No local render client found.'
            )
        return

    global WndInfo

    WndInfo = QtWidgets.QTextEdit()
    WndInfo.setPlainText(json.dumps(render, sort_keys=True, indent=4))
    WndInfo.setReadOnly(True)
    WndInfo.resize(WndInfo.viewport().size())
    WndInfo.setWindowTitle('AFANASY Render Information:')
    WndInfo.show()


def refresh():

    global Render
    global RenderFull

    if Render is None:
        # Get render by local host name
        renders = af.Cmd().renderGetLocal()
        if renders is not None and len(renders):
            Render = renders[0]

    if Render is not None:
        # Get render by ID, as we already know it
        obj = af.Cmd().renderGetId(Render['id'],'full')
        if obj is not None and 'object' in obj and 'render' in obj['object']:
            RenderFull = obj['object']
            Render = RenderFull['render']
            if Refresh:
                Refresh.setDefaultInterval()
        else:
            if obj is None:
                print('ERROR: NULL object reveived.')
            elif 'info' in obj:
                obj = obj['info']
                if obj['kind'] == 'error':
                    print('ERROR: %s' % obj['text'])
                else:
                    print('%s: %s' % (obj['kind'], obj['text']))
            else:
                print('ERROR: Unexpected object reveived:')
                print(json.dumps(RenderFull, sort_keys=True, indent=4))

            # "Reset" render information, if failed to reveive it
            # Most probably render was deleted,
            # and there is no more render with such ID
            Render = None
            RenderFull = None
            # Increase refresh interval by 10 times
            if Refresh:
                Refresh.setIntervalKoeff(10)

    cmd.Tray.showIcon( makeIcon())
    cmd.Tray.updateToolTip(makeTip())


def makeTip():
    if Render is None: return None
    if not 'tasks' in Render: return None
    if len(Render['tasks']) == 0: return None

    tip = ''
    t = 0
    for task in Render['tasks']:
        sec_started = task['time_start']
        percent = Render['tasks_percents'][t]
        sec_running = int(time.time()) - sec_started
        hrs_running = int(sec_running / 3600)
        sec_running = int(sec_running - (hrs_running * 3600))
        mns_running = int(sec_running / 60)
        sec_running = int(sec_running - (mns_running * 60))
        str_running = '%02d:%02d:%02d' % (hrs_running, mns_running, sec_running)

        tip += '\n%s - %s' % (task['user_name'], task['service'])
        tip += '\n%s[%s][%s]' % (task['job_name'], task['block_name'], task['name'])
        tip += '\nRUNNING: %s - %d%%' % (str_running, percent)
        t += 1

    return tip


def makeIcon():

    if Render is None:
        return

    pixmap = QtGui.QPixmap( cmd.Tray.getIconPixmap())
    painter = QtGui.QPainter(pixmap)

    drawIconResources( painter)
    drawIconState( painter)

    painter.end()
    
    return QtGui.QIcon(pixmap)

def drawIconResources( i_painter):
    if RenderFull is None or 'host_resources' not in RenderFull:
        return

    hres = RenderFull['host_resources']
    mem = 1.0 - float(hres['mem_free_mb'])/float(hres['mem_total_mb'])

    factor_min = 0.80
    factor_max = 0.95
    factor = ( mem - factor_min ) / (factor_max - factor_min)
    if factor > 1.0: factor = 1.0
    if factor < 0.0: factor = 0.0
    color_red = int(255 * factor)
    color_green = 255 - color_red
    alpha = int(255 * (.3 + .7*factor))

    icon_size = i_painter.viewport().width()
    x_offset = int(0.1 * icon_size)
    heigth = icon_size * mem
    mem_rect = QtCore.QRect(
        int(x_offset),
        int(icon_size - heigth),
        int(icon_size - (2 * x_offset)),
        int(heigth)
    )
    mem_color = QtGui.QColor( color_red, color_green, 0, alpha)
    i_painter.setBrush( QtGui.QBrush( mem_color))
    i_painter.drawRoundedRect(mem_rect, icon_size*.1, icon_size*.1)

def drawIconState( i_painter):
    if Render is None or 'state' not in Render:
        return

    state = Render['state']

    online = state.find('OFF') == -1
    busy   = state.find('RUN') != -1
    NIMBY  = state.find('NBY') != -1
    nimby  = state.find('NbY') != -1
    paused = state.find('PAU') != -1

    icon_size = i_painter.viewport().width()
    text_font = QtGui.QFont('Arial', int(icon_size / 3))
    text_font.setBold(True)
    rect_back = QtCore.QRect(
        int(icon_size * 3 / 10),
        int(icon_size * 3 / 10),
        int(icon_size * 2 / 5),
        int(icon_size * 2 / 5)
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
                back_color = QtGui.QColor(50,150, 250)
            else:
                text_color = QtGui.QColor(190, 190, 190)
                back_color = QtGui.QColor(40,120, 240)
        elif NIMBY:
            if busy:
                text_color = QtGui.QColor(255, 0, 0)
                back_color = QtGui.QColor(20, 20, 190)
            else:
                text_color = QtGui.QColor(190, 190, 190)
                back_color = QtGui.QColor(10, 10, 150)
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
        int(icon_size / 4),
        int(icon_size / 4),
        int(icon_size / 2),
        int(icon_size / 2)
    )
    i_painter.fillRect(rect_back, back_color)
    i_painter.setFont(text_font)
    i_painter.setPen(text_color)
    i_painter.drawText(rect_render, QtCore.Qt.AlignCenter, 'R')

def setUserDialog():
    name, ok = QtWidgets.QInputDialog.getText(
        None, 'Set Render User', 'Enter New Name:', text=cgruconfig.VARS['USERNAME']
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

