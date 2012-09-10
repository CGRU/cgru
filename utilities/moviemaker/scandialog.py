#!/usr/bin/env python

import os, sys

import cgruconfig
import cgruutils

import cgrupyqt
from cgrupyqt import QtCore, QtGui

# Command arguments:

from optparse import OptionParser
Parser = OptionParser(usage="%prog [options] InputFolder OutputFolder\ntype \"%prog -h\" for help", version="%prog 1.0")
Parser.add_option('-c', '--codec',     dest='codec',        type  ='string',     default='photojpg_best.ffmpeg',help='Default codec preset')
Parser.add_option('-f', '--format',    dest='format',       type  ='string',     default='768x576',   help='Resolution')
Parser.add_option('-t', '--template',  dest='template',     type  ='string',     default='scandpx',   help='Frame paint template')
Parser.add_option('-e', '--extensions',dest='extensions',   type  ='string',     default='dpx,cin',   help='Files extensions, comma searated')
Parser.add_option(      '--include',   dest='include',      type  ='string',     default='',          help='Include path pattern')
Parser.add_option(      '--exclude',   dest='exclude',      type  ='string',     default='',          help='Exclude path pattern')
Parser.add_option('-a', '--abspath',   dest='abspath',      action='store_true', default=False,       help='Prefix movies with images absolute path')
Parser.add_option('-A', '--afanasy',   dest='afanasy',      type  ='int',        default=0,           help='Send commands to Afanasy with specitied capacity')
Parser.add_option('-m', '--maxhosts',  dest='maxhosts',     type  ='int',        default=-1,          help='Afanasy maximum hosts parameter.')
Parser.add_option(      '--pause',     dest='pause',        action='store_true', default=False,       help='Start Afanasy job paused.')
Parser.add_option('--aspect_in',       dest='aspect_in',    type  ='float',      default=-1.0,        help='Input image aspect, -1 = no changes')
Parser.add_option('--aspect_auto',     dest='aspect_auto',  type  ='float',      default=1.2,         help='Auto image aspect (2 if w/h <= aspect_auto), -1 = no changes')
Parser.add_option('-D', '--debug',     dest='debug',        action='store_true', default=False,       help='Debug mode')

Parser.add_option(      '--wndicon',   dest='wndicon',      type  ='string',     default='scanscan',  help='Set dialog window icon filename.')

(Options, args) = Parser.parse_args()
InputFolder  = ''
OutputFolder = '..'
if len(args) > 0: InputFolder  = args[0]
if len(args) > 1: OutputFolder = args[1]

Extensions = ['jpg','dpx','cin','exr','tga','tif','png']

DateTimeFormat = 'yy.MM.dd HH:mm'

# Initializations:
DialogPath = os.path.dirname(os.path.abspath(sys.argv[0]))
TemplatesPath = os.path.join( DialogPath, 'templates')
CodecsPath = os.path.join( DialogPath, 'codecs')
FormatsPath = os.path.join( DialogPath, 'formats')
Encoders = ['ffmpeg', 'mencoder']

# Process formats:
FormatNames = []
FormatValues = []
FormatFiles = []
allFiles = os.listdir( FormatsPath)
for afile in allFiles:
   afile = os.path.join( FormatsPath, afile)
   if os.path.isfile( afile): FormatFiles.append( afile)
FormatFiles.sort()
for afile in FormatFiles:
   file = open( afile)
   FormatNames.append(file.readline().strip())
   FormatValues.append(file.readline().strip())
   file.close()
if not Options.format in FormatValues:
   FormatValues.append( Options.format)
   FormatNames.append( Options.format)

# Process templates:
Templates = ['']
Template = 0
if os.path.isdir(TemplatesPath):
   files = os.listdir(TemplatesPath)
   files.sort()
   index = 0
   for afile in files:
      if afile[0] == '.': continue
      index += 1
      Templates.append(afile)
      if afile == Options.template: Template = index


# Process codecs:
CodecNames = []
CodecFiles = []
allFiles = os.listdir( CodecsPath)
for afile in allFiles:
   afile = os.path.join( CodecsPath, afile)
   if os.path.isfile( afile):
      parts = afile.split('.')
      if len(parts):
         if parts[len(parts)-1] in Encoders:
            CodecFiles.append( afile)
CodecFiles.sort()
for afile in CodecFiles:
   file = open( afile)
   name = file.readline().strip()
   file.close()
   CodecNames.append( name)

class Dialog( QtGui.QWidget):
   def __init__( self):
      QtGui.QWidget.__init__( self)
      self.constructed = False

      self.evaluated = False
      self.test = False

      self.setWindowTitle('Scan Scan - CGRU ' + cgruconfig.VARS['CGRU_VERSION'])
      self.mainLayout = QtGui.QVBoxLayout( self)

      self.tabwidget = QtGui.QTabWidget( self)
      self.generalwidget = QtGui.QWidget( self)
      self.tabwidget.addTab( self.generalwidget,'General')
      self.generallayout = QtGui.QVBoxLayout( self.generalwidget)
      self.parameterswidget = QtGui.QWidget( self)
      self.tabwidget.addTab( self.parameterswidget,'Parameters')
      self.parameterslayout = QtGui.QVBoxLayout( self.parameterswidget)
      self.mainLayout.addWidget( self.tabwidget)


      # General:

      self.lFormat = QtGui.QHBoxLayout()
      self.tFormat = QtGui.QLabel('Format:', self)
      self.tFormat.setToolTip('\
Movie resolution.\n\
Format presets located in\n\
' + FormatsPath)
      self.cbFormat = QtGui.QComboBox( self)
      i = 0
      for format in FormatValues:
         self.cbFormat.addItem( FormatNames[i], format)
         if format == Options.format: self.cbFormat.setCurrentIndex( i)
         i += 1
      QtCore.QObject.connect( self.cbFormat, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.tCodec = QtGui.QLabel('Codec:', self)
      self.tCodec.setToolTip('\
Codec presets located in\n\
' + CodecsPath)
      self.cbCodec = QtGui.QComboBox( self)
      i = 0
      for name in CodecNames:
         self.cbCodec.addItem( name, CodecFiles[i])
         if os.path.basename(CodecFiles[i]) == Options.codec: self.cbCodec.setCurrentIndex(i)
         i = i + 1
      QtCore.QObject.connect( self.cbCodec, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.tFPS = QtGui.QLabel('FPS:', self)
      self.tFPS.setToolTip('\
Frame rate.')
      self.cbFPS = QtGui.QComboBox( self)
      self.cbFPS.addItem('24')
      self.cbFPS.addItem('25')
      self.cbFPS.addItem('30')
      self.cbFPS.setCurrentIndex( 0)
      QtCore.QObject.connect( self.cbFPS, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.lFormat.addWidget( self.tFormat)
      self.lFormat.addWidget( self.cbFormat)
      self.lFormat.addWidget( self.tFPS)
      self.lFormat.addWidget( self.cbFPS)
      self.lFormat.addWidget( self.tCodec)
      self.lFormat.addWidget( self.cbCodec)
      self.generallayout.addLayout( self.lFormat)

      self.lInput = QtGui.QHBoxLayout()
      self.tInput = QtGui.QLabel('Scan Folder', self)
      self.lInput.addWidget( self.tInput)
      self.editInput = QtGui.QLineEdit( InputFolder, self)
      QtCore.QObject.connect( self.editInput, QtCore.SIGNAL('textEdited(QString)'), self.evaluate)
      self.lInput.addWidget( self.editInput)
      self.btnInputBrowse = QtGui.QPushButton('Browse', self)
      QtCore.QObject.connect( self.btnInputBrowse, QtCore.SIGNAL('pressed()'), self.inputBrowse)
      self.lInput.addWidget( self.btnInputBrowse)
      self.generallayout.addLayout( self.lInput)

      self.lExtensions = QtGui.QHBoxLayout()
      self.tExtensions = QtGui.QLabel('Search extensions:', self)
      tooltip = '\
Comma separated list.\n\
Leave empty to find all known:\n'
      for ext in Extensions: tooltip += ext + ' '
      self.tExtensions.setToolTip( tooltip)
      self.lExtensions.addWidget( self.tExtensions)
      self.editExtensions = QtGui.QLineEdit( Options.extensions, self)
      QtCore.QObject.connect( self.editExtensions, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.lExtensions.addWidget( self.editExtensions)
      self.generallayout.addLayout( self.lExtensions)

      self.lInclude = QtGui.QHBoxLayout()
      self.tInclude = QtGui.QLabel('Include pattern:', self)
      self.lInclude.addWidget( self.tInclude)
      self.editInclude = QtGui.QLineEdit( Options.include, self)
      QtCore.QObject.connect( self.editInclude, QtCore.SIGNAL('textEdited(QString)'), self.evaluate)
      self.lInclude.addWidget( self.editInclude)
      self.generallayout.addLayout( self.lInclude)

      self.lExclude = QtGui.QHBoxLayout()
      self.tExclude = QtGui.QLabel('Exclude pattern:', self)
      self.lExclude.addWidget( self.tExclude)
      self.editExclude = QtGui.QLineEdit( Options.exclude, self)
      QtCore.QObject.connect( self.editExclude, QtCore.SIGNAL('textEdited(QString)'), self.evaluate)
      self.lExclude.addWidget( self.editExclude)
      self.generallayout.addLayout( self.lExclude)

      self.lDateTime = QtGui.QHBoxLayout()
      self.cDateTime = QtGui.QCheckBox('Skip folders ealier than:', self)
      QtCore.QObject.connect( self.cDateTime, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.lDateTime.addWidget( self.cDateTime)
      self.eDateTime = QtGui.QDateTimeEdit( QtCore.QDateTime.currentDateTime(), self)
      self.eDateTime.setCalendarPopup( True)
      self.eDateTime.setDisplayFormat( DateTimeFormat)
      QtCore.QObject.connect( self.cDateTime, QtCore.SIGNAL('dateTimeChanged( QDateTime)'), self.evaluate)
      self.lDateTime.addWidget( self.eDateTime)
      self.generallayout.addLayout( self.lDateTime)

      self.lOutput = QtGui.QHBoxLayout()
      self.tOutput = QtGui.QLabel('Output Folder:', self)
      self.lOutput.addWidget( self.tOutput)
      self.editOutput = QtGui.QLineEdit( OutputFolder, self)
      QtCore.QObject.connect( self.editOutput, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.lOutput.addWidget( self.editOutput)
      self.btnOutputBrowse = QtGui.QPushButton('Browse', self)
      QtCore.QObject.connect( self.btnOutputBrowse, QtCore.SIGNAL('pressed()'), self.browseOutput)
      self.lOutput.addWidget( self.btnOutputBrowse)
      self.generallayout.addLayout( self.lOutput)


      # Parameters:

      self.cAbsPath = QtGui.QCheckBox('Prefix movies names with images absolute input files path', self)
      self.cAbsPath.setChecked( Options.abspath)
      QtCore.QObject.connect( self.cAbsPath, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.parameterslayout.addWidget( self.cAbsPath)

      self.lTemplates = QtGui.QHBoxLayout()
      self.tTemplate = QtGui.QLabel('Frame Template:', self)
      self.tTemplate.setToolTip('\
Frame template.\n\
Templates are located in\n\
' + TemplatesPath)
      self.cbTemplate = QtGui.QComboBox( self)
      for template in Templates: self.cbTemplate.addItem(template)
      self.cbTemplate.setCurrentIndex( Template)
      self.lTemplates.addWidget( self.tTemplate)
      self.lTemplates.addWidget( self.cbTemplate)
      QtCore.QObject.connect( self.cbTemplate, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.parameterslayout.addLayout( self.lTemplates)

      self.lAspect = QtGui.QHBoxLayout()
      self.lAspect.addWidget( QtGui.QLabel('Input Images Aspect', self))
      self.dsbAspect = QtGui.QDoubleSpinBox( self)
      self.dsbAspect.setRange( -1.0, 10.0)
      self.dsbAspect.setDecimals( 6)
      self.dsbAspect.setValue( Options.aspect_in)
      QtCore.QObject.connect( self.dsbAspect, QtCore.SIGNAL('valueChanged(double)'), self.evaluate)
      self.lAspect.addWidget( self.dsbAspect)
      self.lAspect.addWidget( QtGui.QLabel(' (-1 = no changes) ', self))
      self.parameterslayout.addLayout( self.lAspect)

      self.lAutoAspect = QtGui.QHBoxLayout()
      self.tAutoAspect = QtGui.QLabel('Auto Input Aspect', self)
      self.tAutoAspect.setToolTip('\
Images with width/height ratio > this value will be treated as 2:1.')
      self.lAutoAspect.addWidget( self.tAutoAspect)
      self.dsbAutoAspect = QtGui.QDoubleSpinBox( self)
      self.dsbAutoAspect.setRange( -1.0, 10.0)
      self.dsbAutoAspect.setDecimals( 3)
      self.dsbAutoAspect.setValue( Options.aspect_auto)
      QtCore.QObject.connect( self.dsbAutoAspect, QtCore.SIGNAL('valueChanged(double)'), self.evaluate)
      self.lAutoAspect.addWidget( self.dsbAutoAspect)
      self.lAutoAspect.addWidget( QtGui.QLabel(' (-1 = no changes) ', self))
      self.parameterslayout.addLayout( self.lAutoAspect)

      self.gCorrectionSettings = QtGui.QGroupBox('Image Correction')
      self.lCorr = QtGui.QHBoxLayout()
      self.gCorrectionSettings.setLayout( self.lCorr)
      self.tGamma = QtGui.QLabel('Gamma:', self)
      self.dsbGamma = QtGui.QDoubleSpinBox( self)
      self.dsbGamma.setRange( 0.1, 10.0)
      self.dsbGamma.setDecimals( 1)
      self.dsbGamma.setSingleStep( 0.1)
      self.dsbGamma.setValue( 1.0)
      QtCore.QObject.connect( self.dsbGamma, QtCore.SIGNAL('valueChanged(double)'), self.evaluate)
      self.lCorr.addWidget( self.tGamma)
      self.lCorr.addWidget( self.dsbGamma)
      self.parameterslayout.addWidget( self.gCorrectionSettings)


      # Bottom tab:

      self.cmdField = QtGui.QTextEdit( self)
      self.cmdField.setReadOnly( True)
      self.mainLayout.addWidget( self.cmdField)

      self.lProcess = QtGui.QHBoxLayout()
      self.btnStart = QtGui.QPushButton('Start', self)
      self.btnStart.setEnabled( False)
      QtCore.QObject.connect( self.btnStart, QtCore.SIGNAL('pressed()'), self.execute)
      self.btnStop = QtGui.QPushButton('Stop', self)
      self.btnStop.setEnabled( False)
      QtCore.QObject.connect( self.btnStop, QtCore.SIGNAL('pressed()'), self.processStop)
      self.btnTest = QtGui.QPushButton('Test', self)
      self.btnTest.setEnabled( False)
      QtCore.QObject.connect( self.btnTest, QtCore.SIGNAL('pressed()'), self.executeTest)
      self.lProcess.addWidget( self.btnTest)
      self.lProcess.addWidget( self.btnStart)
      self.lProcess.addWidget( self.btnStop)
      self.mainLayout.addLayout( self.lProcess)

      self.lAfanasy = QtGui.QHBoxLayout()
      self.cAfanasy = QtGui.QCheckBox('Afanasy', self)
      self.cAfanasy.setChecked( Options.afanasy != 0)
      QtCore.QObject.connect( self.cAfanasy, QtCore.SIGNAL('stateChanged(int)'), self.afanasy)
      self.tAfCapacity = QtGui.QLabel('Capacity:', self)
      self.sbAfCapacity = QtGui.QSpinBox( self)
      self.sbAfCapacity.setRange( -1, 1000000)
      self.sbAfCapacity.setValue( Options.afanasy)
      QtCore.QObject.connect( self.sbAfCapacity, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.tAfMaxHosts = QtGui.QLabel('Maximum Hosts:', self)
      self.sbAfMaxHosts = QtGui.QSpinBox( self)
      self.sbAfMaxHosts.setRange( -1, 1000000)
      self.sbAfMaxHosts.setValue( Options.maxhosts)
      QtCore.QObject.connect( self.sbAfMaxHosts, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.cAfPause = QtGui.QCheckBox('Pause', self)
      self.lAfanasy.addWidget( self.cAfanasy)
      self.lAfanasy.addWidget( self.tAfCapacity)
      self.lAfanasy.addWidget( self.sbAfCapacity)
      self.lAfanasy.addWidget( self.tAfMaxHosts)
      self.lAfanasy.addWidget( self.sbAfMaxHosts)
      self.lAfanasy.addWidget( self.cAfPause)
      self.mainLayout.addLayout( self.lAfanasy)

      self.constructed = True

      self.afanasy()
      self.evaluate()

   def afanasy( self):
      enableAf = self.cAfanasy.isChecked()
      self.sbAfCapacity.setEnabled( enableAf)
      self.sbAfMaxHosts.setEnabled( enableAf)
      if enableAf is True:
         if self.sbAfCapacity.value() == 0: self.sbAfCapacity.setValue( -1 )
      self.evaluate()

   def inputBrowse( self):
      folder = QtGui.QFileDialog.getExistingDirectory( self,'Choose a folder', self.editInput.text())
      if len( folder):
         self.editInput.setText( folder)
         self.evaluate()

   def browseOutput( self):
      folder = QtGui.QFileDialog.getExistingDirectoryy( self,'Choose a folder', self.editOutput.text())
      if len( folder):
         self.editOutput.setText( folder)
         self.evaluate()

   def evaluate( self):
      if not self.constructed: return
      self.evaluated = False
      self.btnStart.setEnabled( False)
      self.btnStop.setEnabled( False)
      self.btnTest.setEnabled( False)

      input = str( self.editInput.text())
      output = str( self.editOutput.text())

      if input == '':
         self.cmdField.setText('Selecet a folder to scan.')
         return
      if not os.path.isdir( input):
         self.cmdField.setText('Input folder does not exist.')
         return
      if output == '':
         output = input
         self.editOutput.setText( output)

      extensions = str( self.editExtensions.text())
      include = str( self.editInclude.text())
      exclude = str( self.editExclude.text())
      format = getComboBoxString( self.cbFormat)
      template = self.cbTemplate.currentText()

      cmd = 'scanscan.py'
      cmd = os.path.join( DialogPath, cmd)
      cmd = '"%s" "%s"' % ( os.getenv('CGRU_PYTHONEXE','python'), cmd)
      cmd += ' -c %s' % getComboBoxString( self.cbCodec)
      cmd += ' -f %s' % self.cbFPS.currentText()
      cmd += ' -r %s' % format
      if self.dsbGamma.value() != 1.0: cmd += ' -g %.2f' % self.dsbGamma.value()
      if template != '': cmd += ' -t "%s"' % template
      if extensions != '': cmd += ' -e "%s"' % extensions
      if include != '': cmd += ' --include "%s"' % include
      if exclude != '': cmd += ' --exclude "%s"' % exclude
      if self.cDateTime.isChecked(): cmd += ' --after %d' % self.eDateTime.dateTime().toTime_t()
      if self.cAbsPath.isChecked(): cmd += ' -a'
      if self.cAfanasy.isChecked():
         cmd += ' -A %d' % self.sbAfCapacity.value()
         cmd += ' -m %d' % self.sbAfMaxHosts.value()
         if self.cAfPause.isChecked(): cmd += ' --pause'
      if self.dsbAspect.value()     > 0: cmd += ' --aspect_in %f' % self.dsbAspect.value()
      if self.dsbAutoAspect.value() > 0: cmd += ' --aspect_auto %f' % self.dsbAutoAspect.value()
      if self.test: cmd += ' --test'

      cmd += ' "%s"' % self.editInput.text()
      cmd += ' "%s"' % self.editOutput.text()

      self.cmdField.setText( cmd)
      self.evaluated = True
      self.btnStart.setEnabled( True)
      self.btnTest.setEnabled( True)

   def executeTest( self):
      if not self.evaluated: return
      self.test = True
      self.execute()

   def execute( self):
      if not self.evaluated: return
      self.evaluate()
      self.command = str( self.cmdField.toPlainText())
      if len( self.command) == 0: return
      self.btnStart.setEnabled( False)
      self.btnStop.setEnabled( True)
      self.btnTest.setEnabled( False)
      self.cmdField.clear()
      self.process = QtCore.QProcess( self)
      self.process.setProcessChannelMode( QtCore.QProcess.MergedChannels)
      QtCore.QObject.connect( self.process, QtCore.SIGNAL('finished( int)'), self.processfinished)
      QtCore.QObject.connect( self.process, QtCore.SIGNAL('readyRead()'), self.processoutput)
      self.process.start( self.command)

   def processfinished( self, exitCode):
      print('Exit code = %d' % exitCode)
      self.btnStop.setEnabled( False)
      if exitCode != 0: return
      if not self.test: self.cmdField.setText( self.command)
      else: self.test = False
      self.btnTest.setEnabled( True)
      self.btnStart.setEnabled( True)

   def processoutput( self):
      output = self.process.readAll().data()
      if not isinstance( output, str): output = str( output, 'utf-8')
      output = output.strip()
      print(output)
      self.cmdField.insertPlainText( output + '\n')
      self.cmdField.moveCursor( QtGui.QTextCursor.End)

   def processStop( self):
      self.process.terminate()


def getComboBoxString( comboBox):
   data = comboBox.itemData( comboBox.currentIndex())
   if data is None: return ''
   if isinstance( data, str): return data
   if isinstance( data, unicode): return data
   return comboBox.itemData( comboBox.currentIndex()).toString()


app = QtGui.QApplication( sys.argv)
app.setWindowIcon( QtGui.QIcon( cgruutils.getIconFileName( Options.wndicon)))
dialog = Dialog()
dialog.show()
app.exec_()
