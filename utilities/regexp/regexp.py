# -*- coding: utf-8 -*-

import sys

import cgruconfig
import cgruutils

from Qt import QtCore, QtGui, QtWidgets, QtCompat


class Dialog(QtWidgets.QWidget):
    def __init__(self):
        QtWidgets.QWidget.__init__(self)
        title = 'Qt RegExp checker   CGRU ' + cgruconfig.VARS['CGRU_VERSION']
        self.setWindowTitle(title)

        layout = QtWidgets.QVBoxLayout(self)

        self.leName = QtWidgets.QLineEdit('render01', self)
        layout.addWidget(self.leName)
        self.leName.textEdited.connect( self.evaluate)

        self.lePattern = QtWidgets.QLineEdit('((render|workstation)0[1-4]+)', self)
        layout.addWidget(self.lePattern)
        self.lePattern.textEdited.connect( self.evaluate)

        self.leResult = QtWidgets.QLineEdit(
            'render01-render04 or workstation01-workstation04', self)
        layout.addWidget(self.leResult)
        self.leResult.setReadOnly(True)
        self.leResult.setAutoFillBackground(True)

        self.resize(500, 100)
        self.show()

    def evaluate(self):
        rx = QtCore.QRegExp(self.lePattern.text())
        if not rx.isValid():
            self.leResult.setText('ERROR: %s' % rx.errorString())
            return
        if rx.exactMatch(self.leName.text()):
            self.leResult.setText('MATCH')
        else:
            self.leResult.setText('NOT MATCH')


app = QtWidgets.QApplication(sys.argv)
app.setWindowIcon( QtGui.QIcon(cgruutils.getIconFileName('regexp')))
dialog = Dialog()
app.exec_()
