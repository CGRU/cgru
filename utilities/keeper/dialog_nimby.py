import cgruconfig

from PyQt4 import QtCore, QtGui

class DialogNimby( QtGui.QWidget):
   def __init__( self, parent = None):
      QtGui.QWidget.__init__( self, parent)
      self.setWindowTitle('Edit Nimby')
      rows = ['day','begin','dash','end','allow','eject']
      self.weekdays = ['Monday','Tuesday','Wednesday','Thursday','Friday','Saturday','Sunday']
      self.days = ['mon','tue','wed','thu','fri','sat','sun']
      self.time_format = 'hh:mm'
      self.te_begin = dict()
      self.te_end = dict()
      self.cb_allow = dict()
      self.cb_eject = dict()

      toplayout = QtGui.QVBoxLayout( self)
      columns = QtGui.QHBoxLayout()
      toplayout.addLayout( columns)
      vlayouts = dict()
      for row in rows:
         vlayouts[row] = QtGui.QVBoxLayout()
         columns.addLayout( vlayouts[row])

      for day in self.days:
         time_begin = '00:00'
         time_end = '00:00'
         allow = True
         eject = False
         var = 'nimby_' + day
         if var in cgruconfig.VARS:
            line = cgruconfig.VARS[var]
            lines = line.split(' ')
            time_begin = lines[0]
            time_end = lines[1]
            allow = False
            eject = False
            if len(lines) > 2:
               if 'a' in lines[2]: allow = True
               if 'e' in lines[2]: eject = True

         vlayouts['day'].addWidget( QtGui.QLabel( day, self))

         self.te_begin[day] = QtGui.QTimeEdit( QtCore.QTime.fromString( time_begin, self.time_format))
         self.te_begin[day].setDisplayFormat( self.time_format)
         vlayouts['begin'].addWidget( self.te_begin[day])

         label = QtGui.QLabel(' - ', self)
         label.setFixedWidth( 16)
         vlayouts['dash'].addWidget( label)

         self.te_end[day] = QtGui.QTimeEdit( QtCore.QTime.fromString( time_end, self.time_format))
         self.te_end[day].setDisplayFormat( self.time_format)
         vlayouts['end'].addWidget( self.te_end[day])

         self.cb_allow[day] = QtGui.QCheckBox('Allow My Jobs', self)
         self.cb_allow[day].setChecked( allow)
         vlayouts['allow'].addWidget( self.cb_allow[day])

         self.cb_eject[day] = QtGui.QCheckBox('Eject Running Tasks', self)
         self.cb_eject[day].setChecked( eject)
         vlayouts['eject'].addWidget( self.cb_eject[day])

      hlayout = QtGui.QHBoxLayout()
      b_accept = QtGui.QPushButton('Accept', self)
      b_cancel = QtGui.QPushButton('Cancel', self)
      hlayout.addWidget( b_accept)
      hlayout.addWidget( b_cancel)
      toplayout.addLayout( hlayout)
      QtCore.QObject.connect( b_accept, QtCore.SIGNAL('pressed()'), self.applySettings)
      QtCore.QObject.connect( b_cancel, QtCore.SIGNAL('pressed()'), self.close)

      self.show()

   def applySettings( self):
      variables = []
      for day in self.days:
         variables.append('nimby_' + day)
         line = self.te_begin[day].time().toString( self.time_format) + ' '
         line += self.te_end[day].time().toString( self.time_format) + ' '
         if self.cb_allow[day].isChecked(): line += 'a'
         if self.cb_eject[day].isChecked(): line += 'e'
         line = line.strip()
         cgruconfig.VARS[variables[-1]] = line
      variables.reverse()
      cgruconfig.writeVars(variables)
      self.close()
