import cgruconfig
import cgruutils

import os

from cgrupyqt import QtCore, QtGui

import nimby

class NimbyDialog( QtGui.QWidget):
   def __init__( self, parent = None):
      QtGui.QWidget.__init__( self, parent)
      self.setWindowTitle('Edit Nimby')
      rows = ['day','begin','dash','end','enable','allow','eject']
      self.weekdays = ['Monday','Tuesday','Wednesday','Thursday','Friday','Saturday','Sunday']
      self.days = ['mon','tue','wed','thu','fri','sat','sun']
      self.time_format = 'hh:mm'
      self.te_begin = dict()
      self.te_end = dict()
      self.cb_enable = dict()
      self.cb_allow = dict()
      self.cb_eject = dict()

      toplayout = QtGui.QVBoxLayout( self)
      columns = QtGui.QHBoxLayout()
      toplayout.addLayout( columns)
      vlayouts = dict()
      for row in rows:
         vlayouts[row] = QtGui.QVBoxLayout()
         columns.addLayout( vlayouts[row])

      daynum = 0
      for day in self.days:
         time_begin = '00:00'
         time_end = '00:00'
         enable = False
         allow = True
         eject = False
         var = 'nimby_' + day
         if var in cgruconfig.VARS:
            line = cgruconfig.VARS[var]
            if line is None: continue
            lines = line.split(' ')
            if len(lines) < 2: continue
            time_begin = lines[0]
            time_end = lines[1]
            enable = False
            allow = True
            eject = False
            if 'nimby'  in lines: allow  = True
            if 'NIMBY'  in lines: allow  = False
            if 'Eject'  in lines: eject  = True
            if 'Enable' in lines: enable = True

         vlayouts['day'].addWidget( QtGui.QLabel( self.weekdays[daynum], self))

         self.te_begin[day] = QtGui.QTimeEdit( QtCore.QTime.fromString( time_begin, self.time_format))
         self.te_begin[day].setDisplayFormat( self.time_format)
         vlayouts['begin'].addWidget( self.te_begin[day])

         label = QtGui.QLabel(' - ', self)
         label.setFixedWidth( 16)
         vlayouts['dash'].addWidget( label)

         self.te_end[day] = QtGui.QTimeEdit( QtCore.QTime.fromString( time_end, self.time_format))
         self.te_end[day].setDisplayFormat( self.time_format)
         vlayouts['end'].addWidget( self.te_end[day])

         self.cb_enable[day] = QtGui.QCheckBox('Enable', self)
         self.cb_enable[day].setChecked( enable)
         vlayouts['enable'].addWidget( self.cb_enable[day])

         self.cb_allow[day] = QtGui.QCheckBox('Allow My Jobs', self)
         self.cb_allow[day].setChecked( allow)
         vlayouts['allow'].addWidget( self.cb_allow[day])

         self.cb_eject[day] = QtGui.QCheckBox('Eject Running Tasks', self)
         self.cb_eject[day].setChecked( eject)
         vlayouts['eject'].addWidget( self.cb_eject[day])

         daynum += 1

      hlayout = QtGui.QHBoxLayout()
      b_accept = QtGui.QPushButton('Accept', self)
      b_cancel = QtGui.QPushButton('Cancel', self)
      hlayout.addWidget( b_accept)
      hlayout.addWidget( b_cancel)
      toplayout.addLayout( hlayout)
      QtCore.QObject.connect( b_accept, QtCore.SIGNAL('pressed()'), self.applySettings)
      QtCore.QObject.connect( b_cancel, QtCore.SIGNAL('pressed()'), self.close)

      # Set window icon:
      iconpath = cgruutils.getIconFileName('afanasy')
      if iconpath is not None: self.setWindowIcon( QtGui.QIcon( iconpath))

      self.show()

   def applySettings( self):
      variables = []
      for day in self.days:
         variables.append('nimby_' + day)
         line =  str( self.te_begin[day].time().toString( self.time_format)) + ' '
         line += str( self.te_end[day].time().toString( self.time_format))
         if self.cb_enable[day].isChecked(): line += ' Enable'
         if self.cb_allow[day].isChecked(): line += ' nimby'
         else: line += ' NIMBY'
         if self.cb_eject[day].isChecked(): line += ' Eject'
         line = line.strip()
         cgruconfig.VARS[variables[-1]] = line
      variables.reverse()
      cgruconfig.writeVars(variables)
      nimby.refresh( reset = True)
      self.close()
