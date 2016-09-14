# -*- coding: utf-8 -*-

import subprocess
import sys

import cgruconfig
import cgruutils

import cmd
import nimby
import render
import tray

from cgrupyqt import QtCore, QtNetwork


class Server(QtNetwork.QTcpServer):
    def __init__(self, application):
        QtNetwork.QTcpServer.__init__(self, application)
        self.parent = application
        self.setMaxPendingConnections(1)
        self.port = int(cgruconfig.VARS['keeper_port'])
        QtCore.QObject.connect(
            self,
            QtCore.SIGNAL('newConnection()'),
            self.connection
        )
        if not self.listen(QtNetwork.QHostAddress(QtNetwork.QHostAddress.Any), self.port):
            print('Can`t listen %d port.' % self.port)
        else:
            print('Listening %d port...' % self.port)
        self.qsocket = None

    def connection(self):
        if not self.hasPendingConnections():
            return
        while True:
            qsocket = self.nextPendingConnection()
            if qsocket is None:
                break

            print('Server connected...')
            self.qsocket = qsocket

            QtCore.QObject.connect(
                self.qsocket,
                QtCore.SIGNAL('readyRead()'),
                self.readCommand
            )

    def readCommand(self):

        print('Server reading...')

        if self.qsocket is None:
            return

        isHTTP = False
        content_len = 0
        content_len_str = 'Content-Length: '
        command = None

        while True:

            line = cgruutils.toStr( self.qsocket.readLine().data())

            if len( line) == 0:
                break

            #sys.stdout.write(line)

            if line.find('HTTP') != -1:
                isHTTP = True
                continue
            elif not isHTTP:
                command = line
                break

            if line.find( content_len_str ) == 0:
                content_len = int(line[len(content_len_str):])
                if content_len == 0:
                    break
                continue

            if isHTTP:
                if line == '\r\n':
                    command = cgruutils.toStr( self.qsocket.readLine( content_len).data())
                    break
                else:
                    continue


        if command is None:
            return


        if not isHTTP:
            command = 'cmd.%s()' % command
            print('Executing: "%s"' % command)
            eval(command)
            return

        print('Executing:')
        print( command)

        self.qsocket.write('HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK')

        self.qsocket.disconnectFromHost()

        subprocess.Popen( command, shell=True)

