import cgruconfig

from PyQt4 import QtCore, QtGui

class DialogNimby( QtGui.QWidget):
   def __init__( self, parent = None):
      QtGui.QWidget.__init__( self, parent)
      self.setWindowTitle('Edit Nimby')

      vlayout = QtGui.QVBoxLayout( self)

      hlayout = QtGui.QHBoxLayout()
      hlayout.addWidget( QtGui.QLabel('Monday: ', self))
      self.te_mon = QtGui.QTimeEdit()
      self.te_mon.setDisplayFormat('hh:mm')
      hlayout.addWidget( self.te_mon)
      vlayout.addLayout( hlayout)

      hlayout = QtGui.QHBoxLayout()
      b_accept = QtGui.QPushButton('Accept', self)
      b_cancel = QtGui.QPushButton('Cancel', self)
      hlayout.addWidget( b_accept)
      hlayout.addWidget( b_cancel)
      vlayout.addLayout( hlayout)
      QtCore.QObject.connect( b_cancel, QtCore.SIGNAL('pressed()'), self.close)

      self.show()
