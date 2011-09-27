import os, sys, time

from PyQt4 import QtCore, QtGui

from optparse import OptionParser
Parser = OptionParser(usage="usage: %prog [options] hip_name rop_name", version="%prog 1.0")
Parser.add_option('-V', '--verbose',    action='store_true', dest='verbose', default=False, help='Verbose mode')
(Options, Args) = Parser.parse_args()

# Initializations:
Scene = ''
if len(Args) > 0: Scene = Args[0]

# Dialog class
class Dialog( QtGui.QWidget):
   def __init__( self):
      QtGui.QWidget.__init__( self)
      self.setWindowTitle('Afanasy Starter   CGRU ' + os.environ['CGRU_VERSION'])
      
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
      self.leScene = QtGui.QLineEdit( Scene, self)
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
      labelNode = QtGui.QLabel('Node/Camera:')
      lNode.addWidget( labelNode)
      labelNode.setToolTip('\
Houdini ROP\n\
Nuke write node\n\
3DSMAX camera')
      self.leNode = QtGui.QLineEdit( self)
      lNode.addWidget( self.leNode)
      QtCore.QObject.connect( self.leNode, QtCore.SIGNAL('textEdited(QString)'), self.evaluate)
      labelTake = QtGui.QLabel('Take/Pass/Batch:')
      lNode.addWidget( labelTake)
      labelTake.setToolTip('\
Houdini take\n\
SoftImage pass\n\
3DSMAX batch')
      self.leTake = QtGui.QLineEdit( self)
      lNode.addWidget( self.leTake)
      QtCore.QObject.connect( self.leTake, QtCore.SIGNAL('textEdited(QString)'), self.evaluate)


      # Job:
      lJobName = QtGui.QHBoxLayout()
      joblayout.addLayout( lJobName)
      lJobName.addWidget( QtGui.QLabel('Name:', self))
      self.leJobName = QtGui.QLineEdit( self)
      lJobName.addWidget( self.leJobName)
      QtCore.QObject.connect( self.leJobName, QtCore.SIGNAL('textEdited(QString)'), self.evaluate)
      self.cbJobName = QtGui.QCheckBox('Use Scene Name', self)
      lJobName.addWidget( self.cbJobName)
      self.cbJobName.setChecked( True)
      QtCore.QObject.connect( self.cbJobName, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)

      # Capacity, max run tasks, priority:
      lCapMax = QtGui.QHBoxLayout()
      joblayout.addLayout( lCapMax)
      lCapMax.addWidget( QtGui.QLabel('Capacity:', self))
      self.sbCapacity = QtGui.QSpinBox( self)
      lCapMax.addWidget( self.sbCapacity)
      self.sbCapacity.setRange(-1, 1000000)
      self.sbCapacity.setValue(-1)
      QtCore.QObject.connect( self.sbCapacity, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      lCapMax.addWidget( QtGui.QLabel('Maximum Running Tasks:', self))
      self.sbMaxRunTasks = QtGui.QSpinBox( self)
      lCapMax.addWidget( self.sbMaxRunTasks)
      self.sbMaxRunTasks.setRange(-1, 1000000)
      self.sbMaxRunTasks.setValue(-1)
      QtCore.QObject.connect( self.sbMaxRunTasks, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      lCapMax.addWidget( QtGui.QLabel('Priority:', self))
      self.sbPriority = QtGui.QSpinBox( self)
      lCapMax.addWidget( self.sbPriority)
      self.sbPriority.setRange(-1, 250)
      self.sbPriority.setValue(-1)
      QtCore.QObject.connect( self.sbPriority, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)

      # Depend Masks:
      lDepends = QtGui.QHBoxLayout()
      joblayout.addLayout( lDepends)
      lDepends.addWidget( QtGui.QLabel('Depend Mask:', self))
      self.leDependMask = QtGui.QLineEdit( self)
      lDepends.addWidget( self.leDependMask)
      QtCore.QObject.connect( self.leDependMask, QtCore.SIGNAL('textEdited(QString)'), self.evaluate)
      lDepends.addWidget( QtGui.QLabel('Global:', self))
      self.leDependGlobal = QtGui.QLineEdit( self)
      lDepends.addWidget( self.leDependGlobal)
      QtCore.QObject.connect( self.leDependGlobal, QtCore.SIGNAL('textEdited(QString)'), self.evaluate)

      # Host Masks:
      lHostMasks = QtGui.QHBoxLayout()
      joblayout.addLayout( lHostMasks)
      lHostMasks.addWidget( QtGui.QLabel('Hosts Mask:', self))
      self.leHostsMask = QtGui.QLineEdit( self)
      lHostMasks.addWidget( self.leHostsMask)
      QtCore.QObject.connect( self.leHostsMask, QtCore.SIGNAL('textEdited(QString)'), self.evaluate)
      lHostMasks.addWidget( QtGui.QLabel('Exclude:', self))
      self.leHostsExclude = QtGui.QLineEdit( self)
      lHostMasks.addWidget( self.leHostsExclude)
      QtCore.QObject.connect( self.leHostsExclude, QtCore.SIGNAL('textEdited(QString)'), self.evaluate)

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
      if self.cbJobName.isChecked():
         self.leJobName.setText( os.path.basename( str( self.leScene.text())))
         self.leJobName.setEnabled( False)
      else:
         self.leJobName.setEnabled( True)

      # Check scene:
      if len( self.leScene.text()) == 0: return
      if not os.path.isfile( self.leScene.text()):
         self.teCmd.setText('Scene file does not exist.')
         return
      self.leScene.setText( os.path.abspath( str( self.leScene.text())))

      # Check working directory:
      if self.cbWDir.isChecked(): self.leWDir.setText( os.path.dirname( str( self.leScene.text())))
      if not os.path.isdir( self.leWDir.text()):
         self.teCmd.setText('Working directory does not exist.')
         return
      self.leWDir.setText( os.path.abspath( str( self.leWDir.text())))

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
      if self.sbCapacity.value() > 0: cmd += ' -capacity %d' % self.sbCapacity.value()
      if self.sbMaxRunTasks.value() > 0: cmd += ' -maxruntasks %d' % self.sbMaxRunTasks.value()
      if self.sbPriority.value() > -1: cmd += ' -priority %d' % self.sbPriority.value()
      if not self.leDependMask.text().isEmpty(): cmd += ' -depmask "%s"' % self.leDependMask.text()
      if not self.leDependGlobal.text().isEmpty(): cmd += ' -depglbl "%s"' % self.leDependGlobal.text()
      if not self.leHostsMask.text().isEmpty(): cmd += ' -hostsmask "%s"' % self.leHostsMask.text()
      if not self.leHostsExclude.text().isEmpty(): cmd += ' -hostsexcl "%s"' % self.leHostsExclude.text()
      if self.cbPaused.isChecked(): cmd += ' -pause'
      if not self.cbJobName.isChecked() and not self.leJobName.text().isEmpty(): cmd += ' -name "%s"' % self.leJobName.text()

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
