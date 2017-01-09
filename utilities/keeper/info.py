# -*- coding: utf-8 -*-

import sys
import os

import cgruconfig
import cgruutils

from Qt import QtCore, QtGui, QtWidgets, QtCompat

class Window(QtWidgets.QTextEdit):
    def __init__(self, parent=None):
        QtWidgets.QTextEdit.__init__(self, parent)

        self.setWindowTitle(
            'Configuration: %s version %s' % (
                cgruconfig.VARS['company'],
                cgruconfig.VARS['CGRU_VERSION']
            )
        )

        self.ftitle = QtGui.QTextCharFormat()
        self.ftitle.setFontWeight( QtGui.QFont.Bold)
        self.ftitle.setFontItalic(True)
        self.fname = QtGui.QTextCharFormat()
        self.fname.setFontWeight( QtGui.QFont.Bold)
        self.fvalue = QtGui.QTextCharFormat()
        self.fundefined = QtGui.QTextCharFormat()
        self.fundefined.setFontItalic(True)

        self.appendVar('Platform', ",".join( cgruconfig.VARS['platform']))

        self.textCursor().insertText('\nPython:\n', self.ftitle)
        self.appendVar('Qt', '%s %s-%s' % ( QtCompat.__qt_version__, QtCompat.__binding__, QtCompat.__binding_version__))
        self.appendVar('sys.prefix', sys.prefix)
        self.appendVar('Executable', os.getenv('CGRU_PYTHONEXE'))
        self.appendVar('Version', sys.version)

        self.textCursor().insertText('\nEnvironment:\n', self.ftitle)
        self.appendEnvVar('PYTHONHOME')
        self.appendEnvVar('PYTHONPATH')
        self.appendEnvVar('CGRU_PYTHON')
        self.appendEnvVar('AF_ROOT')
        self.appendEnvVar('AF_PYTHON')
        self.appendEnvVar('AF_RENDER_CMD')
        self.appendEnvVar('AF_WATCH_CMD')
        self.appendVar('PWD', os.getcwd())
        self.appendEnvVar('PATH')

        self.textCursor().insertText('\nVariables:\n', self.ftitle)

        self.appendVars(cgruconfig.VARS)

        for afile in cgruconfig.VARS['filenames']:
            self.appendConfigFile(afile)

        # Set window icon:
        iconpath = cgruutils.getIconFileName('info')
        if iconpath is not None:
            self.setWindowIcon( QtGui.QIcon(iconpath))

        self.resize(self.viewport().size())
        self.moveCursor( QtGui.QTextCursor.Start)
        self.setReadOnly(True)
        self.show()

    def appendConfigFile(self, filename):
        self.textCursor().insertText('\n%s:\n' % filename, self.ftitle)
        variables = dict()
        cgruconfig.Config(variables, [filename])
        self.appendVars(variables)

    def appendVars(self, variables):
        keys = []
        for key in variables.keys():
            keys.append(key)
        keys.sort()
        for var in keys:
            self.appendVar(var, variables[var])

    def appendEnvVar(self, name):
        self.appendVar(name, os.getenv(name))

    def appendVar(self, name, value=None):
        c = self.textCursor()
        c.insertText(name, self.fname)
        if value is None:
            c.insertText(' - ', self.fvalue)
            c.insertText('undefined', self.fundefined)
        else:
            c.insertText(' = ', self.fvalue)
            c.insertText(str(value), self.fvalue)
        c.insertText('\n')
