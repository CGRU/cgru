import os, sys

import cgruconfig
import cgruutils

from cgrupyqt import QtCore, QtGui

class Dialog( QtGui.QWidget):
   def __init__( self):
      QtGui.QWidget.__init__( self)
      title = 'Qt RegExp checker   CGRU ' + cgruconfig.VARS['CGRU_VERSION']
      self.setWindowTitle( title)

      layout = QtGui.QVBoxLayout( self)

      self.leName = QtGui.QLineEdit('render01', self)
      layout.addWidget( self.leName)
      QtCore.QObject.connect( self.leName, QtCore.SIGNAL('textEdited(QString)'), self.evaluate)

      self.lePattern = QtGui.QLineEdit('((render|workstation)0[1-4]{1,})', self)
      layout.addWidget( self.lePattern)
      QtCore.QObject.connect( self.lePattern, QtCore.SIGNAL('textEdited(QString)'), self.evaluate)

      self.leResult = QtGui.QLineEdit('render01-render04 or workstation01-workstation04', self)
      layout.addWidget( self.leResult)
      self.leResult.setReadOnly( True);
      self.leResult.setAutoFillBackground( True);

      self.resize( 500 , 100)
      self.show()

   def evaluate( self):
      rx = QtCore.QRegExp( self.lePattern.text())
      if not rx.isValid():
         self.leResult.setText( 'ERROR: %s' % rx.errorString())
         return
      if rx.exactMatch( self.leName.text()):
         self.leResult.setText('MATCH')
      else:
         self.leResult.setText('NOT MATCH')

app = QtGui.QApplication( sys.argv)
app.setWindowIcon( QtGui.QIcon( cgruutils.getIconFileName('regexp')))
dialog = Dialog()
app.exec_()
