#!/usr/bin/env python

import os, sys

from PyQt4 import QtCore, QtGui

# Command arguments:

from optparse import OptionParser
Parser = OptionParser(usage="%prog [options]\ntype \"%prog -h\" for help", version="%prog 1.0")
Parser.add_option('-i', '--input',     dest='input',     type  ='string',     default='',          help='Input folder to scan')
Parser.add_option('-o', '--output',    dest='output',    type  ='string',     default='',          help='Output folder for movies')
Parser.add_option('-t', '--template',  dest='template',  type  ='string',     default='scandpx',   help='Frame paint template')
Parser.add_option('-e', '--extensions',dest='extensions',type  ='string',     default='',          help='Files extensions, comma searated')
Parser.add_option('-a', '--abspath',   dest='abspath',   action='store_true', default=False,       help='Prefix movies with images absolute path')
Parser.add_option('-A', '--afanasy',   dest='afanasy',   type  ='int',        default=0,           help='Send commands to Afanasy with specitied capacity')
Parser.add_option('-m', '--maxhosts',  dest='maxhosts',  type  ='int',        default=-1,          help='Afanasy maximum hosts parameter.')
Parser.add_option('-D', '--debug',     dest='debug',     action='store_true', default=False,       help='Debug mode')

(Options, args) = Parser.parse_args()

# Initializations:
DialogPath = os.path.dirname(os.path.abspath(sys.argv[0]))
TemplatesPath = os.path.join( DialogPath, 'templates')
CodecsPath = DialogPath
Encoders = ['ffmpeg', 'mencoder']

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
      self.evaluated = False

      self.setWindowTitle('Scan Scan')
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
      self.cbFormat = QtGui.QComboBox( self)
      self.cbFormat.addItem('PAL (720x576)', QtCore.QVariant('720x576'))
      self.cbFormat.addItem('PAL Square (768x576)', QtCore.QVariant('768x576'))
      self.cbFormat.addItem('HD 720p (1280x720)', QtCore.QVariant('1280x720'))
      self.cbFormat.addItem('HD 1080p (1920x1080)', QtCore.QVariant('1920x1080'))
      self.cbFormat.setCurrentIndex( 1)
      QtCore.QObject.connect( self.cbFormat, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.tCodec = QtGui.QLabel('Codec:', self)
      self.cbCodec = QtGui.QComboBox( self)
      i = 0
      for name in CodecNames:
         self.cbCodec.addItem( name, QtCore.QVariant( CodecFiles[i]))
         i = i + 1
      QtCore.QObject.connect( self.cbCodec, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.tFPS = QtGui.QLabel('FPS:', self)
      self.cbFPS = QtGui.QComboBox( self)
      self.cbFPS.addItem('24')
      self.cbFPS.addItem('25')
      self.cbFPS.addItem('30')
      self.cbFPS.setCurrentIndex( 1)
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
      self.editInput = QtGui.QLineEdit( Options.input, self)
      QtCore.QObject.connect( self.editInput, QtCore.SIGNAL('textEdited(QString)'), self.evaluate)
      self.lInput.addWidget( self.editInput)
      self.btnInputBrowse = QtGui.QPushButton('Browse', self)
      QtCore.QObject.connect( self.btnInputBrowse, QtCore.SIGNAL('pressed()'), self.inputBrowse)
      self.lInput.addWidget( self.btnInputBrowse)
      self.generallayout.addLayout( self.lInput)

      self.lOutput = QtGui.QHBoxLayout()
      self.tOutput = QtGui.QLabel('Output Folder:', self)
      self.lOutput.addWidget( self.tOutput)
      self.editOutput = QtGui.QLineEdit( Options.output, self)
      QtCore.QObject.connect( self.editOutput, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.lOutput.addWidget( self.editOutput)
      self.btnOutputBrowse = QtGui.QPushButton('Browse', self)
      QtCore.QObject.connect( self.btnOutputBrowse, QtCore.SIGNAL('pressed()'), self.browseOutput)
      self.lOutput.addWidget( self.btnOutputBrowse)
      self.generallayout.addLayout( self.lOutput)

      self.cAbsPath = QtGui.QCheckBox('Prefix movies names with images absolute path', self)
      self.cAbsPath.setChecked( Options.abspath)
      QtCore.QObject.connect( self.cAbsPath, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.generallayout.addWidget( self.cAbsPath)

      # Parameters:
      self.lTemplates = QtGui.QHBoxLayout()
      self.tTemplate = QtGui.QLabel('Frame Template:', self)
      self.cbTemplate = QtGui.QComboBox( self)
      for template in Templates: self.cbTemplate.addItem(template)
      self.cbTemplate.setCurrentIndex( Template)
      self.lTemplates.addWidget( self.tTemplate)
      self.lTemplates.addWidget( self.cbTemplate)
      QtCore.QObject.connect( self.cbTemplate, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.parameterslayout.addLayout( self.lTemplates)

      self.lExtensions = QtGui.QHBoxLayout()
      self.tExtensions1 = QtGui.QLabel('Search extensions:', self)
      self.lExtensions.addWidget( self.tExtensions1)
      self.editExtensions = QtGui.QLineEdit( Options.extensions, self)
      QtCore.QObject.connect( self.editExtensions, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.lExtensions.addWidget( self.editExtensions)
      self.tExtensions2 = QtGui.QLabel('(comma separated list, empty - find all known)', self)
      self.lExtensions.addWidget( self.tExtensions2)
      self.parameterslayout.addLayout( self.lExtensions)

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
      self.lProcess.addWidget( self.btnStop)
      self.lProcess.addWidget( self.btnStart)
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
      self.lAfanasy.addWidget( self.cAfanasy)
      self.lAfanasy.addWidget( self.tAfCapacity)
      self.lAfanasy.addWidget( self.sbAfCapacity)
      self.lAfanasy.addWidget( self.tAfMaxHosts)
      self.lAfanasy.addWidget( self.sbAfMaxHosts)
      self.mainLayout.addLayout( self.lAfanasy)

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
      if folder.isEmpty(): return
      self.editInput.setText( folder)
      self.evaluate()

   def browseOutput( self):
      folder = QtGui.QFileDialog.getExistingDirectory( self,'Choose a folder', self.editOutput.text())
      if folder.isEmpty(): return
      self.editOutput.setText( folder)
      self.evaluate()

   def evaluate( self):
      self.evaluated = False

      input = str( self.editInput.text())
      output = str( self.editOutput.text())
      
      if input == '':
         self.cmdField.setText('Selecet a folder to scan.')
         return
      if not os.path.isdir( input):
         self.cmdField.setText('Input folder does not exist.')
         return
      if output == '':
         self.cmdField.setText('Selecet a folder for movies.')
         return
      if not os.path.isdir( output):
         self.cmdField.setText('Ouput folder does not exist.')
         return

      extensions = str( self.editExtensions.text())
      format = self.cbFormat.itemData( self.cbFormat.currentIndex()).toString()
      template = self.cbTemplate.currentText()

      cmd = 'scanscan.py'
      cmd = 'python ' + os.path.join( DialogPath, cmd)
      cmd += ' -c %s' % self.cbCodec.itemData( self.cbCodec.currentIndex()).toString()
      cmd += ' -f %s' % self.cbFPS.currentText()
      cmd += ' -r %s' % format
      if self.dsbGamma.value() != 1.0: cmd += ' -g %.2f' % self.dsbGamma.value()
      if template != '': cmd += ' -t "%s"' % template
      if extensions != '': cmd += ' -e "%s"' % extensions
      if self.cAbsPath.isChecked(): cmd += ' -a'
      if self.cAfanasy.isChecked():
         cmd += ' -A %d' % self.sbAfCapacity.value()
         cmd += ' -m %d' % self.sbAfMaxHosts.value()
      cmd += ' "%s"' % self.editInput.text()
      cmd += ' "%s"' % self.editOutput.text()

      self.cmdField.setText( cmd)
      self.evaluated = True
      self.btnStart.setEnabled( True)

   def execute( self):
      if not self.evaluated: return
      self.command = str( self.cmdField.toPlainText())
      if len( self.command) == 0: return
      self.btnStart.setEnabled( False)
      self.btnStop.setEnabled( True)
      self.cmdField.clear()
      self.process = QtCore.QProcess( self)
      self.process.setProcessChannelMode( QtCore.QProcess.MergedChannels)
      QtCore.QObject.connect( self.process, QtCore.SIGNAL('finished( int)'), self.processfinished)
      QtCore.QObject.connect( self.process, QtCore.SIGNAL('readyRead()'), self.processoutput)
      self.process.start( self.command)

   def processfinished( self, exitCode):
      print 'Exit code = %d' % exitCode
      self.btnStop.setEnabled( False)
      if exitCode != 0: return
      self.cmdField.setText( self.command)
      self.btnStart.setEnabled( True)

   def processoutput( self):
      output = str( self.process.readAll())
      print output,
      self.cmdField.insertPlainText( output)
      self.cmdField.moveCursor( QtGui.QTextCursor.End)

   def processStop( self):
      self.process.terminate()

app = QtGui.QApplication( sys.argv)
icon = QtGui.QIcon( os.path.join( os.path.join (DialogPath, 'icons'), 'icon.png'))
app.setWindowIcon( icon)
dialog = Dialog()
dialog.show()
app.exec_()
