import cgruconfig

from PyQt4 import QtCore, QtGui

class DialogNimby( QtGui.QWidget):
   def __init__( self, parent = None):
      QtGui.QWidget.__init__( self, parent)
      self.setWindowTitle('Edit Nimby')
      self.days = ['Monday','Tuesday','Wednesday','Thursday','Friday','Saturday','Sunday']
      self.te_begin = dict()
      self.te_end = dict()

      vlayout = QtGui.QVBoxLayout( self)

      for day in self.days:
         hlayout = QtGui.QHBoxLayout()
         hlayout.addWidget( QtGui.QLabel( day, self))
         self.te_begin[day] = QtGui.QTimeEdit()
         self.te_begin[day].setDisplayFormat('hh:mm')
         hlayout.addWidget( self.te_begin[day])
         self.te_end[day] = QtGui.QTimeEdit()
         self.te_end[day].setDisplayFormat('hh:mm')
         hlayout.addWidget( self.te_end[day])
         vlayout.addLayout( hlayout)

      hlayout = QtGui.QHBoxLayout()
      b_accept = QtGui.QPushButton('Accept', self)
      b_cancel = QtGui.QPushButton('Cancel', self)
      hlayout.addWidget( b_accept)
      hlayout.addWidget( b_cancel)
      vlayout.addLayout( hlayout)
      QtCore.QObject.connect( b_cancel, QtCore.SIGNAL('pressed()'), self.close)

      self.show()
