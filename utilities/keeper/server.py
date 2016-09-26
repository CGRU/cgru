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

        if self.qsocket is None:
            return

        print('Server reading...')

        cmd.execute( cgruutils.toStr( self.qsocket.readAll().data()))

