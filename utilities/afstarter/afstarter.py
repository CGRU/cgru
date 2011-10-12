import os, sys, time

from PyQt4 import QtCore, QtGui

import cgruconfig

import afcommon

from optparse import OptionParser
Parser = OptionParser(usage="usage: %prog [options] hip_name rop_name", version="%prog 1.0")
Parser.add_option('-V', '--verbose',    action='store_true', dest='verbose', default=False, help='Verbose mode')
(Options, Args) = Parser.parse_args()

# Initializations:
Scene = ''
if len(Args) > 0: Scene = Args[0]
FilePrefix = 'afstarter.'
FileSuffix = '.txt'
FileLast = 'last'
FileRecent = 'recent'

# Dialog class
class Dialog( QtGui.QWidget):
   def __init__( self):
      QtGui.QWidget.__init__( self)
      self.setWindowTitle('Afanasy Starter   CGRU ' + os.environ['CGRU_VERSION'])
      self.fields = dict()
      
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
      self.fields['scenefile'] = QtGui.QLineEdit( Scene, self)
      lScene.addWidget( self.fields['scenefile'])
      QtCore.QObject.connect( self.fields['scenefile'], QtCore.SIGNAL('editingFinished()'), self.evaluate)
      bBrowseScene = QtGui.QPushButton('Browse', self)
      lScene.addWidget( bBrowseScene)
      QtCore.QObject.connect( bBrowseScene, QtCore.SIGNAL('pressed()'), self.browseScene)

      # Working Directory:
      lWDir = QtGui.QHBoxLayout()
      generallayout.addLayout( lWDir)
      lWDir.addWidget( QtGui.QLabel('Working Directory/Project:', self))
      self.fields['wdir'] = QtGui.QLineEdit( self)
      lWDir.addWidget( self.fields['wdir'])
      QtCore.QObject.connect( self.fields['wdir'], QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.fields['scenewdir'] = QtGui.QCheckBox('Use Scene Folder', self)
      self.fields['scenewdir'].setChecked( True)
      QtCore.QObject.connect( self.fields['scenewdir'], QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      lWDir.addWidget( self.fields['scenewdir'])

      # Output images:
      lImages = QtGui.QHBoxLayout()
      generallayout.addLayout( lImages)
      lImages.addWidget( QtGui.QLabel('Output images:', self))
      self.fields['outimages'] = QtGui.QLineEdit( self)
      lImages.addWidget( self.fields['outimages'])
      QtCore.QObject.connect( self.fields['outimages'], QtCore.SIGNAL('editingFinished()'), self.evaluate)
      bBrowseOutImages = QtGui.QPushButton('Browse', self)
      lImages.addWidget( bBrowseOutImages)
      QtCore.QObject.connect( bBrowseOutImages, QtCore.SIGNAL('pressed()'), self.browseOutImages)

      # Frames:
      lFrames = QtGui.QHBoxLayout()
      generallayout.addLayout( lFrames)
      lFrames.addWidget( QtGui.QLabel('Frames:', self))
      self.fields['framestart'] = QtGui.QSpinBox( self)
      self.fields['framestart'].setRange( -1000000000, 1000000000)
      self.fields['framestart'].setValue(1)
      QtCore.QObject.connect( self.fields['framestart'], QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      lFrames.addWidget( self.fields['framestart'])
      self.fields['frameend'] = QtGui.QSpinBox( self)
      self.fields['frameend'].setRange( -1000000000, 1000000000)
      self.fields['frameend'].setValue(1)
      QtCore.QObject.connect( self.fields['frameend'], QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      lFrames.addWidget( self.fields['frameend'])
      lFrames.addWidget( QtGui.QLabel('by', self))
      self.fields['frameby'] = QtGui.QSpinBox( self)
      lFrames.addWidget( self.fields['frameby'])
      QtCore.QObject.connect( self.fields['frameby'], QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.fields['frameby'].setRange( 1, 1000000000)
      lFrames.addWidget( QtGui.QLabel('per task', self))
      self.fields['framespt'] = QtGui.QSpinBox( self)
      lFrames.addWidget( self.fields['framespt'])
      QtCore.QObject.connect( self.fields['framespt'], QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.fields['framespt'].setRange( 1, 1000000000)


      # Node / Camera / Take:
      lNode = QtGui.QHBoxLayout()
      generallayout.addLayout( lNode)
      labelNode = QtGui.QLabel('Node/Camera:')
      lNode.addWidget( labelNode)
      labelNode.setToolTip('\
Houdini ROP\n\
Nuke write node\n\
Maya camera\n\
3DSMAX camera')
      self.fields['node'] = QtGui.QLineEdit( self)
      lNode.addWidget( self.fields['node'])
      QtCore.QObject.connect( self.fields['node'], QtCore.SIGNAL('textEdited(QString)'), self.evaluate)
      labelTake = QtGui.QLabel('Take/Layer/Pass/Batch:')
      lNode.addWidget( labelTake)
      labelTake.setToolTip('\
Houdini take\n\
SoftImage pass\n\
Maya layer\n\
3DSMAX batch')
      self.fields['take'] = QtGui.QLineEdit( self)
      lNode.addWidget( self.fields['take'])
      QtCore.QObject.connect( self.fields['take'], QtCore.SIGNAL('textEdited(QString)'), self.evaluate)


      # Job:
      lJobName = QtGui.QHBoxLayout()
      joblayout.addLayout( lJobName)
      lJobName.addWidget( QtGui.QLabel('Name:', self))
      self.fields['jobname'] = QtGui.QLineEdit( self)
      lJobName.addWidget( self.fields['jobname'])
      QtCore.QObject.connect( self.fields['jobname'], QtCore.SIGNAL('textEdited(QString)'), self.evaluate)
      self.fields['jobnamescene'] = QtGui.QCheckBox('Use Scene Name', self)
      lJobName.addWidget( self.fields['jobnamescene'])
      self.fields['jobnamescene'].setChecked( True)
      QtCore.QObject.connect( self.fields['jobnamescene'], QtCore.SIGNAL('stateChanged(int)'), self.evaluate)

      # Capacity, max run tasks, priority:
      lCapMax = QtGui.QHBoxLayout()
      joblayout.addLayout( lCapMax)
      lCapMax.addWidget( QtGui.QLabel('Capacity:', self))
      self.fields['capacity'] = QtGui.QSpinBox( self)
      lCapMax.addWidget( self.fields['capacity'])
      self.fields['capacity'].setRange(-1, 1000000)
      self.fields['capacity'].setValue(-1)
      QtCore.QObject.connect( self.fields['capacity'], QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      lCapMax.addWidget( QtGui.QLabel('Maximum Running Tasks:', self))
      self.fields['maxruntasks'] = QtGui.QSpinBox( self)
      lCapMax.addWidget( self.fields['maxruntasks'])
      self.fields['maxruntasks'].setRange(-1, 1000000)
      self.fields['maxruntasks'].setValue(-1)
      QtCore.QObject.connect( self.fields['maxruntasks'], QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      lCapMax.addWidget( QtGui.QLabel('Priority:', self))
      self.fields['priority'] = QtGui.QSpinBox( self)
      lCapMax.addWidget( self.fields['priority'])
      self.fields['priority'].setRange(-1, 250)
      self.fields['priority'].setValue(-1)
      QtCore.QObject.connect( self.fields['priority'], QtCore.SIGNAL('valueChanged(int)'), self.evaluate)

      # Depend Masks:
      lDepends = QtGui.QHBoxLayout()
      joblayout.addLayout( lDepends)
      lDepends.addWidget( QtGui.QLabel('Depend Mask:', self))
      self.fields['dependmask'] = QtGui.QLineEdit( self)
      lDepends.addWidget( self.fields['dependmask'])
      QtCore.QObject.connect( self.fields['dependmask'], QtCore.SIGNAL('textEdited(QString)'), self.evaluate)
      lDepends.addWidget( QtGui.QLabel('Global:', self))
      self.fields['dependglobal'] = QtGui.QLineEdit( self)
      lDepends.addWidget( self.fields['dependglobal'])
      QtCore.QObject.connect( self.fields['dependglobal'], QtCore.SIGNAL('textEdited(QString)'), self.evaluate)

      # Host Masks:
      lHostMasks = QtGui.QHBoxLayout()
      joblayout.addLayout( lHostMasks)
      lHostMasks.addWidget( QtGui.QLabel('Hosts Mask:', self))
      self.fields['hostsmask'] = QtGui.QLineEdit( self)
      lHostMasks.addWidget( self.fields['hostsmask'])
      QtCore.QObject.connect( self.fields['hostsmask'], QtCore.SIGNAL('textEdited(QString)'), self.evaluate)
      lHostMasks.addWidget( QtGui.QLabel('Exclude:', self))
      self.fields['hostsexclude'] = QtGui.QLineEdit( self)
      lHostMasks.addWidget( self.fields['hostsexclude'])
      QtCore.QObject.connect( self.fields['hostsexclude'], QtCore.SIGNAL('textEdited(QString)'), self.evaluate)

      # Presets:
      presetsLayout = QtGui.QHBoxLayout()
      topLayout.addLayout( presetsLayout)
      presetsLayout.addWidget( QtGui.QLabel('Recent:', self))
      self.cbRecent = QtGui.QComboBox( self)
      presetsLayout.addWidget( self.cbRecent)
      QtCore.QObject.connect( self.cbRecent, QtCore.SIGNAL('currentIndexChanged(int)'), self.loadRecent)
      self.bBrowseLoad = QtGui.QPushButton('Load', self)
      presetsLayout.addWidget( self.bBrowseLoad)
      QtCore.QObject.connect( self.bBrowseLoad, QtCore.SIGNAL('pressed()'), self.browseLoad)
      self.bBrowseSave = QtGui.QPushButton('Save', self)
      presetsLayout.addWidget( self.bBrowseSave)
      QtCore.QObject.connect( self.bBrowseSave, QtCore.SIGNAL('pressed()'), self.browseSave)


      # Command Field:
      self.teCmd = QtGui.QTextEdit( self)
      topLayout.addWidget( self.teCmd)


      # Buttons:
      buttonsLayout = QtGui.QHBoxLayout()
      topLayout.addLayout( buttonsLayout)
      self.bStart = QtGui.QPushButton('&Start', self)
      buttonsLayout.addWidget( self.bStart)
      QtCore.QObject.connect( self.bStart, QtCore.SIGNAL('pressed()'), self.start)
      self.fields['paused'] = QtGui.QCheckBox('Paused', self)
      buttonsLayout.addWidget( self.fields['paused'])
      QtCore.QObject.connect( self.fields['paused'], QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.bRefresh = QtGui.QPushButton('&Refresh', self)
      buttonsLayout.addWidget( self.bRefresh)
      QtCore.QObject.connect( self.bRefresh, QtCore.SIGNAL('pressed()'), self.evaluate)
      self.bQuitSave = QtGui.QPushButton('&Quit&&Store', self)
      buttonsLayout.addWidget( self.bQuitSave)
      QtCore.QObject.connect( self.bQuitSave, QtCore.SIGNAL('pressed()'), self.quitsave)

      # Refresh recent:
      self.refreshRecent()

      # Load last settings:
      if not self.load( FileLast): self.evaluate()

   def browseScene( self):
      scene = str( QtGui.QFileDialog.getOpenFileName( self,'Choose a file', self.fields['scenefile'].text()))
      if scene == '': return
      self.fields['scenefile'].setText( scene)
      self.evaluate()

   def browseOutImages( self):
      path = str( QtGui.QFileDialog.getOpenFileName( self,'Choose a file', self.fields['outimages'].text()))
      if path == '': return
      self.fields['outimages'].setText( path)
      self.evaluate()

   def quitsave( self):
      self.save( FileLast)
      self.close()

   def save( self, filename, fullPath = False):
      if not fullPath: filename = os.path.join( cgruconfig.VARS['HOME_CGRU'], FilePrefix) + filename + FileSuffix
      file = open( filename,'w')
      for key in self.fields:
         value = ''
         if isinstance( self.fields[key], QtGui.QLineEdit):
            value = str( self.fields[key].text())
         elif isinstance( self.fields[key], QtGui.QSpinBox):
            value = str( self.fields[key].value())
         elif isinstance( self.fields[key], QtGui.QCheckBox):
            value = str( int( self.fields[key].isChecked()))
         line = key + '=' + value
         file.write( line + '\n')
      file.close()

   def getRecentFilesList( self):
      allfiles = os.listdir( cgruconfig.VARS['HOME_CGRU'])
      recfiles = []
      for afile in allfiles:
         if afile.find( FilePrefix + FileRecent) >= 0: recfiles.append( afile)
      recfiles.sort()
      return recfiles

   def saveRecent( self):
      recfiles = self.getRecentFilesList()
      if len(recfiles) > 0:
         for afile in recfiles:
            if afile.find( self.fields['jobname'].text()) > len(FilePrefix + FileRecent):
#               print('os.remove("%s")' % os.path.join( cgruconfig.VARS['HOME_CGRU'], afile))
               os.remove( os.path.join( cgruconfig.VARS['HOME_CGRU'], afile))
               recfiles.remove( afile)
         numfiles = len(recfiles)
         if numfiles > 9:
#            print('os.remove("%s")' % os.path.join( cgruconfig.VARS['HOME_CGRU'], recfiles[-1]))
            os.remove( os.path.join( cgruconfig.VARS['HOME_CGRU'], recfiles[-1]))
            del recfiles[-1]
         recfiles.reverse()
         index = len(recfiles)
         for afile in recfiles:
            pos = afile.find( FilePrefix + FileRecent)
            if pos < 0: continue
            pos = len(FilePrefix + FileRecent)
            num = int(afile[pos])
            if num != index:
               nextfile = afile[:pos] + str(index) + afile[pos+1:]
               afile = os.path.join( cgruconfig.VARS['HOME_CGRU'], afile)
               nextfile = os.path.join( cgruconfig.VARS['HOME_CGRU'], nextfile)
#               print('os.rename("%s"->"%s")' % ( afile, nextfile))
               os.rename( afile, nextfile)
            index -= 1
      afile = FileRecent + '0.' + self.fields['jobname'].text()
      self.save( afile)
      self.refreshRecent()

   def refreshRecent( self):
      self.cbRecent.clear()
      for afile in self.getRecentFilesList():
         afile = afile.replace( FilePrefix,'')
         afile = afile.replace( FileSuffix,'')
         short = afile.replace( FileRecent,'')[2:]
         if len(short) > 20: short = short[:10] + ' .. ' + short[-10:]
         self.cbRecent.addItem( short, afile)

   def loadRecent( self):
      if self.load( str( self.cbRecent.itemData( self.cbRecent.currentIndex()).toString())): self.evaluate()

   def load( self, filename, fullPath = False):
      if not fullPath: filename = os.path.join( cgruconfig.VARS['HOME_CGRU'], FilePrefix) + filename + FileSuffix
      if not os.path.isfile( filename): return False
      file = open( filename,'r')
      lines = file.readlines()
      file.close()
      for line in lines:
         pos = line.find('=')
         if pos < 1: continue
         key = line[:pos]
         if key not in self.fields: continue
         value = line[pos+1:].strip()         
         if isinstance( self.fields[key], QtGui.QLineEdit):
            self.fields[key].setText( value)
         elif isinstance( self.fields[key], QtGui.QSpinBox):
            self.fields[key].setValue( int(value))
         elif isinstance( self.fields[key], QtGui.QCheckBox):
            self.fields[key].setChecked( int(value))
      self.evaluate()
      return True

   def browseLoad( self):
      filename = str( QtGui.QFileDialog.getOpenFileName( self,'Choose afstarter file', cgruconfig.VARS['HOME_CGRU']))
      self.load( filename, True)

   def browseSave( self):
      filename = str( QtGui.QFileDialog.getSaveFileName( self,'Choose afstarter file', cgruconfig.VARS['HOME_CGRU']))
      self.save( filename, True)

   def evaluate( self):
      self.evaluated = False
      self.bStart.setEnabled( False)

      # Check parameters:
      if self.fields['framestart'].value() > self.fields['frameend'].value(): self.fields['frameend'].setValue( self.fields['framestart'].value())
      if self.fields['scenewdir'].isChecked(): self.fields['wdir'].setEnabled( False)
      else: self.fields['wdir'].setEnabled( True)
      if self.fields['jobnamescene'].isChecked():
         self.fields['jobname'].setText( os.path.basename( str( self.fields['scenefile'].text())))
         self.fields['jobname'].setEnabled( False)
      else:
         self.fields['jobname'].setEnabled( True)

      # Check scene:
      scene = str( self.fields['scenefile'].text())
      if len( scene) == 0: return
      if scene[-4:] == '.ifd':
         scene = afcommon.patternFromFile( scene)
      elif not os.path.isfile( scene):
         self.teCmd.setText('Scene file does not exist.')
         return
      scene = os.path.normpath( scene)
      scene = os.path.abspath( scene)
      self.fields['scenefile'].setText( scene)

      # Check working directory:
      if self.fields['scenewdir'].isChecked(): self.fields['wdir'].setText( os.path.dirname( str( self.fields['scenefile'].text())))
      if not os.path.isdir( self.fields['wdir'].text()):
         self.teCmd.setText('Working directory does not exist.')
         return
      self.fields['wdir'].setText( os.path.abspath( str( self.fields['wdir'].text())))

      # Check job name:
      if str( self.fields['jobname'].text()) == '':
         self.teCmd.setText('Job name is empty.')
         return

      # Construct command:
      cmd = os.environ['AF_ROOT']
      cmd = os.path.join( cmd, 'python')
      cmd = os.path.join( cmd, 'afjob.py')
      cmd = 'python ' + cmd
      cmd += ' "%s"' % self.fields['scenefile'].text()
      cmd += ' %d' % self.fields['framestart'].value()
      cmd += ' %d' % self.fields['frameend'].value()
      cmd += ' -by %d' % self.fields['frameby'].value()
      cmd += ' -fpt %d' % self.fields['framespt'].value()
      if not str( self.fields['node'].text()) == '': cmd += ' -node "%s"' % self.fields['node'].text()
      if not str( self.fields['take'].text()) == '': cmd += ' -take "%s"' % self.fields['take'].text()
      if not str( self.fields['outimages'].text()) == '': cmd += ' -output "%s"' % self.fields['outimages'].text()
      cmd += ' -pwd "%s"' % self.fields['wdir'].text()
      if self.fields['capacity'].value() > 0: cmd += ' -capacity %d' % self.fields['capacity'].value()
      if self.fields['maxruntasks'].value() > 0: cmd += ' -maxruntasks %d' % self.fields['maxruntasks'].value()
      if self.fields['priority'].value() > -1: cmd += ' -priority %d' % self.fields['priority'].value()
      if not str( self.fields['dependmask'].text()) == '': cmd += ' -depmask "%s"' % self.fields['dependmask'].text()
      if not str( self.fields['dependglobal'].text()) == '': cmd += ' -depglbl "%s"' % self.fields['dependglobal'].text()
      if not str( self.fields['hostsmask'].text()) == '': cmd += ' -hostsmask "%s"' % self.fields['hostsmask'].text()
      if not str( self.fields['hostsexclude'].text()) == '': cmd += ' -hostsexcl "%s"' % self.fields['hostsexclude'].text()
      if self.fields['paused'].isChecked(): cmd += ' -pause'
      cmd += ' -name "%s"' % self.fields['jobname'].text()

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
      self.saveRecent()

   def processoutput( self):
      output = str( self.process.readAll())
      print( output)
      self.teCmd.insertPlainText( output)
      self.teCmd.moveCursor( QtGui.QTextCursor.End)

app = QtGui.QApplication( sys.argv)
dialog = Dialog()
dialog.show()
app.exec_()
