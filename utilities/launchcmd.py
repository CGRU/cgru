#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys

if len(sys.argv) <= 1:
    print('Error: No command specified.')
    sys.exit(1)

from Qt import QtCore, QtGui, QtWidgets


class Dialog(QtWidgets.QWidget):
    def __init__(self):
        QtWidgets.QWidget.__init__(self)

        self.setWindowTitle(sys.argv[0])

        layout = QtWidgets.QVBoxLayout(self)
        self.cmdField = QtWidgets.QTextEdit(self)
        self.cmdField.setReadOnly(True)
        layout.addWidget(self.cmdField)
        self.outputField = QtWidgets.QTextEdit(self)
        self.outputField.setReadOnly(True)
        layout.addWidget(self.outputField)

        command = ''
        for arg in sys.argv[1:]:
            if command != '':
                command += ' '
            command += '"%s"' % arg
        if sys.version_info[0] < 3:
            command = command.decode('utf-8')
        else:
            command = str(command, 'utf-8')
        self.cmdField.setText(command)

        arguments = []
        if sys.platform.find('win') == 0:
            shell = 'cmd.exe'
            arguments.append('/c')
        else:
            shell = '/bin/bash'
            arguments.append('-c')
        arguments.append(command)

        self.process = QtCore.QProcess(self)
        self.process.setProcessChannelMode(QtCore.QProcess.MergedChannels)
        self.process.finished.connect( self.processfinished)
        self.process.readyRead.connect( self.processoutput)
        self.process.start(shell, arguments)

    def closeEvent(self, event):
        self.process.terminate()
        if sys.platform.find('win') == 0:
            self.process.kill()
        self.outputField.setText('Stopped.')
        self.process.waitForFinished()

    def processfinished(self, exitCode):
        print('Exit code = %d' % exitCode)
        if exitCode == 0:
            self.outputField.insertPlainText(
                'Command finished successfully.')

    def processoutput(self):
        output = self.process.readAll()
        if not isinstance(output, str):
            if sys.version_info[0] < 3:
                output = '%s' % output
            else:
                output = str(output, 'utf-8')
        output = output.strip()
        print('%s' % output)
        self.outputField.insertPlainText(output + '\n')
        self.outputField.moveCursor( QtGui.QTextCursor.End)


app = QtWidgets.QApplication(sys.argv)
dialog = Dialog()
dialog.show()
app.exec_()
