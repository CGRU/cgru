import cgruconfig

import os, sys

from PyQt4 import QtCore, QtGui

class Window( QtGui.QTextEdit ):
   def __init__( self, parent = None):
      QtGui.QWidget.__init__( self, parent)

      self.ftitle = QtGui.QTextCharFormat()
      self.ftitle.setFontWeight( QtGui.QFont.Bold)
      self.ftitle.setFontItalic( True)
      self.fname = QtGui.QTextCharFormat()
      self.fname.setFontWeight( QtGui.QFont.Bold)
      self.fvalue = QtGui.QTextCharFormat()
      self.fundefined = QtGui.QTextCharFormat()
      self.fundefined.setFontItalic( True)

      self.textCursor().insertText('Variables:\n', self.ftitle)

      self.appendVars( cgruconfig.VARS)

      self.textCursor().insertText('\nAFANASY Environment:\n', self.ftitle)
      self.appendEnvVar('AF_ROOT')
      self.appendEnvVar('AF_RENDER_CMD')
      self.appendEnvVar('AF_WATCH_CMD')

      self.appendConfigFile( cgruconfig.VARS['CONFIGFILE'])
      self.appendConfigFile( cgruconfig.VARS['HOME_CONFIGFILE'])

      cmd = ''
      for arg in sys.argv: cmd += ' ' + arg
      self.setWindowTitle('Configuration:' + cmd)
      self.resize( self.viewport().size())
      self.moveCursor( QtGui.QTextCursor.Start)
      self.setReadOnly( True)
      self.show()

   def appendConfigFile( self, filename):
      self.textCursor().insertText( '\n%s:\n' % filename, self.ftitle)
      variables = dict()
      cgruconfig.Config( variables, [filename])
      self.appendVars( variables)

   def appendVars( self, variables):
      keys = []
      for key in variables.keys(): keys.append( key)
      keys.sort()
      for var in keys:
         self.appendVar( var, variables[var])

   def appendEnvVar( self, name): self.appendVar( name, os.getenv( name))

   def appendVar( self, name, value = None):
      c = self.textCursor()
      c.insertText( name, self.fname)
      if value is None:
         c.insertText( ' - ', self.fvalue)
         c.insertText( 'undefined', self.fundefined)
      else:
         c.insertText( ' = ', self.fvalue)
         c.insertText( value, self.fvalue)
      c.insertText( '\n')
