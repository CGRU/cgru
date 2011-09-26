import os, sys, time

from PyQt4 import QtCore, QtGui

class Dialog( QtGui.QWidget):
   def __init__( self):
      QtGui.QWidget.__init__( self)
      self.setWindowTitle('Afanasy Starter')
      
      topLayout = QtGui.QVBoxLayout( self)
      tabwidget = QtGui.QTabWidget( self)
      topLayout.addWidget( tabwidget)

      generalwidget = QtGui.QWidget( self)
      tabwidget.addTab( generalwidget,'Scene')
      generallayout = QtGui.QVBoxLayout( generalwidget)

      jobwidget = QtGui.QWidget( self)
      tabwidget.addTab( jobwidget,'Job')
      joblayout = QtGui.QVBoxLayout( jobwidget)


      # Scene:
      lScene = QtGui.QHBoxLayout()
      generallayout.addLayout( lScene)
      lScene.addWidget( QtGui.QLabel('File:', self))
      self.leScene = QtGui.QLineEdit( self)
      lScene.addWidget( self.leScene)
      QtCore.QObject.connect( self.leScene, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      bBrowseScene = QtGui.QPushButton('Browse', self)
      lScene.addWidget( bBrowseScene)
      QtCore.QObject.connect( bBrowseScene, QtCore.SIGNAL('pressed()'), self.browseScene)

      # Working Directory:
      lWDir = QtGui.QHBoxLayout()
      generallayout.addLayout( lWDir)
      lWDir.addWidget( QtGui.QLabel('Working Directory:', self))
      self.leWDir = QtGui.QLineEdit( self)
      lWDir.addWidget( self.leWDir)
      QtCore.QObject.connect( self.leWDir, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.cbWDir = QtGui.QCheckBox('Use Scene Folder', self)
      self.cbWDir.setChecked( True)
      QtCore.QObject.connect( self.cbWDir, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      lWDir.addWidget( self.cbWDir)


      # Frames:
      lFrames = QtGui.QHBoxLayout()
      generallayout.addLayout( lFrames)
      lFrames.addWidget( QtGui.QLabel('Frames:', self))
      self.sbFrameStart = QtGui.QSpinBox( self)
      self.sbFrameStart.setRange( -1000000000, 1000000000)
      self.sbFrameStart.setValue(1)
      QtCore.QObject.connect( self.sbFrameStart, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      lFrames.addWidget( self.sbFrameStart)
      self.sbFrameEnd = QtGui.QSpinBox( self)
      self.sbFrameEnd.setRange( -1000000000, 1000000000)
      self.sbFrameEnd.setValue(1)
      QtCore.QObject.connect( self.sbFrameEnd, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      lFrames.addWidget( self.sbFrameEnd)
      lFrames.addWidget( QtGui.QLabel('by', self))
      self.sbFrameBy = QtGui.QSpinBox( self)
      lFrames.addWidget( self.sbFrameBy)
      QtCore.QObject.connect( self.sbFrameBy, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.sbFrameBy.setRange( 1, 1000000000)
      lFrames.addWidget( QtGui.QLabel('per task', self))
      self.sbFramePT = QtGui.QSpinBox( self)
      lFrames.addWidget( self.sbFramePT)
      QtCore.QObject.connect( self.sbFramePT, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.sbFramePT.setRange( 1, 1000000000)


      # Node / Camera / Take:
      lNode = QtGui.QHBoxLayout()
      generallayout.addLayout( lNode)
      lNode.addWidget( QtGui.QLabel('Node/Camera:', self))
      self.leNode = QtGui.QLineEdit( self)
      lNode.addWidget( self.leNode)
      QtCore.QObject.connect( self.leNode, QtCore.SIGNAL('textEdited(QString)'), self.evaluate)
      lNode.addWidget( QtGui.QLabel('Take/Pass:', self))
      self.leTake = QtGui.QLineEdit( self)
      lNode.addWidget( self.leTake)
      QtCore.QObject.connect( self.leTake, QtCore.SIGNAL('textEdited(QString)'), self.evaluate)


      # Command Field:

      self.teCmd = QtGui.QTextEdit( self)
      topLayout.addWidget( self.teCmd)


      # Buttons:

      buttonsLayout = QtGui.QHBoxLayout()
      topLayout.addLayout( buttonsLayout)
      self.bStart = QtGui.QPushButton('Start', self)
      buttonsLayout.addWidget( self.bStart)
      QtCore.QObject.connect( self.bStart, QtCore.SIGNAL('pressed()'), self.start)
      self.cbPaused = QtGui.QCheckBox('Paused', self)
      buttonsLayout.addWidget( self.cbPaused)
      QtCore.QObject.connect( self.cbPaused, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.bRefresh = QtGui.QPushButton('Refresh', self)
      buttonsLayout.addWidget( self.bRefresh)
      QtCore.QObject.connect( self.bRefresh, QtCore.SIGNAL('pressed()'), self.evaluate)

      self.evaluate()

   def browseScene( self):
      scene = QtGui.QFileDialog.getOpenFileName( self,'Choose a file', self.leScene.text())
      if scene.isEmpty(): return
      self.leScene.setText( scene)
      self.evaluate()

   def evaluate( self):
      self.evaluated = False
      self.bStart.setEnabled( False)

      # Check parameters:
      if self.sbFrameStart.value() > self.sbFrameEnd.value(): self.sbFrameEnd.setValue( self.sbFrameStart.value())
      if self.cbWDir.isChecked(): self.leWDir.setEnabled( False)
      else: self.leWDir.setEnabled( True)

      # Check scene:
      if len( self.leScene.text()) == 0: return
      if not os.path.isfile( self.leScene.text()):
         self.teCmd.setText('Scene file does not exist.')
         return

      # Check working directory:
      if self.cbWDir.isChecked(): self.leWDir.setText( os.path.dirname( str(self.leScene.text())))
      if not os.path.isdir( self.leWDir.text()):
         self.teCmd.setText('Working directory does not exist.')
         return

      # Construct command:
      cmd = os.environ['AF_ROOT']
      cmd = os.path.join( cmd, 'python')
      cmd = os.path.join( cmd, 'afjob.py')
      cmd = 'python ' + cmd
      cmd += ' "%s"' % self.leScene.text()
      cmd += ' %d' % self.sbFrameStart.value()
      cmd += ' %d' % self.sbFrameEnd.value()
      cmd += ' -by %d' % self.sbFrameBy.value()
      cmd += ' -fpt %d' % self.sbFramePT.value()
      if not self.leNode.text().isEmpty(): cmd += ' -node "%s"' % self.leNode.text()
      if not self.leTake.text().isEmpty(): cmd += ' -take "%s"' % self.leTake.text()
      cmd += ' -pwd "%s"' % self.leWDir.text()
      if self.cbPaused.isChecked(): cmd += ' -pause'

      # Evaluated:
      self.teCmd.setText( cmd)
      self.evaluated = True
      self.bStart.setEnabled( True)


   def start( self):
      self.evaluate()
      if not self.evaluated: return
      self.bStart.setEnabled( False)
      command = str( self.teCmd.toPlainText())
      if len( command) == 0: return
      self.teCmd.clear()
      time.sleep(.1)
      self.process = QtCore.QProcess( self)
      self.process.setProcessChannelMode( QtCore.QProcess.MergedChannels)
      QtCore.QObject.connect( self.process, QtCore.SIGNAL('finished( int)'), self.processfinished)
      QtCore.QObject.connect( self.process, QtCore.SIGNAL('readyRead()'), self.processoutput)
      self.process.start( command)

   def processfinished( self, exitCode):
      print('Exit code = %d' % exitCode)
      if exitCode != 0: return
      self.bStart.setEnabled( True)

   def processoutput( self):
      output = str( self.process.readAll())
      print( output)
      self.teCmd.insertPlainText( output)
      self.teCmd.moveCursor( QtGui.QTextCursor.End)

app = QtGui.QApplication( sys.argv)
dialog = Dialog()
dialog.show()
app.exec_()
