import cgruconfig
import cmd
import nimby
import render
import tray

from cgrupyqt import QtCore, QtNetwork

class Server( QtNetwork.QTcpServer):
   def __init__( self, application):
      QtNetwork.QTcpServer.__init__( self, application)
      self.parent = application
      self.setMaxPendingConnections(1)
      self.port = int( cgruconfig.VARS['keeper_port'])
      QtCore.QObject.connect( self, QtCore.SIGNAL('newConnection()'), self.connection)
      if not self.listen( QtNetwork.QHostAddress(QtNetwork.QHostAddress.Any), self.port):
         print('Can`t listen %d port.' % self.port)
      else:
         print('Listening %d port...' % self.port)
      self.qsocket = None

   def connection( self):
      if not self.hasPendingConnections(): return
      while True:
         qsocket = self.nextPendingConnection()
         if qsocket is None: break
         print('Server connected...')
         self.qsocket = qsocket
         QtCore.QObject.connect( self.qsocket, QtCore.SIGNAL('readChannelFinished()'), self.readCommand)

   def readCommand( self):
      print('Server reading...')
      if self.qsocket is None: return
      c = '%s' % self.qsocket.readAll().data()
      print('Server: "%s"' % c)
      c = c.replace('b\'','')
      c = c.replace('\\r\\n\'','')
      c = c.replace('\\n\'','')
      c = c.strip('\'')
      if len( c) < 1: return
      c = 'cmd.' + c + '()'
      print('Executing:')
      print( c)
      eval( c)
