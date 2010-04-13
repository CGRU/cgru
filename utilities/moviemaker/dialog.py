#!/usr/bin/env python
# -*- coding: utf-8 -*-

from datetime import datetime
import os
import re
import subprocess
import sys

from PyQt4 import QtCore, QtGui

class Dialog( QtGui.QWidget):
   def __init__( self):
      QtGui.QWidget.__init__( self)
      self.evaluated = False

      self.setWindowTitle('Mavishky !')
      self.mainLayout = QtGui.QVBoxLayout( self)

      self.lFormat = QtGui.QHBoxLayout()
      self.tFormat = QtGui.QLabel('Format:', self)
      self.cbFormat = QtGui.QComboBox( self)
      self.cbFormat.addItem('try "as is"')
      self.cbFormat.addItem('PAL (720x576)', QtCore.QVariant('720x576'))
      self.cbFormat.addItem('PAL Square (768x576)', QtCore.QVariant('768x576'))
      self.cbFormat.addItem('HD 720p (1280x720)', QtCore.QVariant('1280x720'))
      self.cbFormat.addItem('HD 1080p (1920x1080)', QtCore.QVariant('1920x1080'))
      self.cbFormat.setCurrentIndex( 1)
      QtCore.QObject.connect( self.cbFormat, QtCore.SIGNAL('currentIndexChanged(int)'), self.formatChanged)
      self.tCodec = QtGui.QLabel('Codec:', self)
      self.cbCodec = QtGui.QComboBox( self)
      self.cbCodec.addItem('Quicktime (PhotoJPG)', QtCore.QVariant('mov'))
      self.cbCodec.addItem('Mpeg (MPEG4 XVID)', QtCore.QVariant('mpeg'))
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
      self.mainLayout.addLayout( self.lFormat)


      self.gDrawing = QtGui.QGroupBox('Drawing')
      self.lDrawing = QtGui.QVBoxLayout()
      self.gDrawing.setLayout( self.lDrawing)

      self.lCacher = QtGui.QHBoxLayout()
      self.tCacherH = QtGui.QLabel('16:9 Cacher', self)
      self.cbCacherH = QtGui.QComboBox( self)
      self.cbCacherH.addItem('None')
      self.cbCacherH.addItem('50%', QtCore.QVariant('50'))
      self.cbCacherH.addItem('100%', QtCore.QVariant('100'))
      QtCore.QObject.connect( self.cbCacherH, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.tCacherC = QtGui.QLabel('2.35 Cacher', self)
      self.cbCacherC = QtGui.QComboBox( self)
      self.cbCacherC.addItem('None')
      self.cbCacherC.addItem('50%', QtCore.QVariant('50'))
      self.cbCacherC.addItem('100%', QtCore.QVariant('100'))
      QtCore.QObject.connect( self.cbCacherC, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.lCacher.addWidget( self.tCacherH)
      self.lCacher.addWidget( self.cbCacherH)
      self.lCacher.addWidget( self.tCacherC)
      self.lCacher.addWidget( self.cbCacherC)
      self.lDrawing.addLayout( self.lCacher)

      self.lDraw = QtGui.QHBoxLayout()
      self.cFrame = QtGui.QCheckBox('Draw Frame', self)
      QtCore.QObject.connect( self.cFrame, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.cDate = QtGui.QCheckBox('Draw Date', self)
      QtCore.QObject.connect( self.cDate, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.cTitle = QtGui.QCheckBox('Draw Title', self)
      QtCore.QObject.connect( self.cTitle, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.cLogo = QtGui.QCheckBox('Draw Logo', self)
      QtCore.QObject.connect( self.cLogo, QtCore.SIGNAL('stateChanged(int)'), self.drawLogo)
      self.lDraw.addWidget( self.cFrame)
      self.lDraw.addWidget( self.cDate)
      self.lDraw.addWidget( self.cTitle)
      self.lDraw.addWidget( self.cLogo)
      self.lDrawing.addLayout( self.lDraw)

      self.lTitles = QtGui.QHBoxLayout()
      self.tTitle = QtGui.QLabel('Title:', self)
      self.editTitle = QtGui.QLineEdit('', self)
      QtCore.QObject.connect( self.editTitle, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.cAutoTitle = QtGui.QCheckBox('Auto', self)
      self.cAutoTitle.setChecked( True)
      QtCore.QObject.connect( self.cAutoTitle, QtCore.SIGNAL('stateChanged(int)'), self.autoTitle)
      self.tAnnotate = QtGui.QLabel('Annotate:', self)
      self.editAnnotate = QtGui.QLineEdit( self)
      QtCore.QObject.connect( self.editAnnotate, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.lTitles.addWidget( self.tTitle)
      self.lTitles.addWidget( self.editTitle)
      self.lTitles.addWidget( self.cAutoTitle)
      self.lTitles.addWidget( self.tAnnotate)
      self.lTitles.addWidget( self.editAnnotate)
      self.lDrawing.addLayout( self.lTitles)

      self.lLogo = QtGui.QHBoxLayout()
      self.tLogoPath = QtGui.QLabel('Logo Path:', self)
      self.editLogoPath = QtGui.QLineEdit('/cg/common/logo.png', self)
      QtCore.QObject.connect( self.editLogoPath, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.tLogoSize = QtGui.QLabel('Logo Size:', self)
      self.sbLogoSizeX = QtGui.QSpinBox( self)
      self.sbLogoSizeX.setRange( 1, 1000)
      self.sbLogoSizeX.setValue( 200)
      QtCore.QObject.connect( self.sbLogoSizeX, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.sbLogoSizeY = QtGui.QSpinBox( self)
      self.sbLogoSizeY.setRange( 1, 1000)
      self.sbLogoSizeY.setValue( 100)
      QtCore.QObject.connect( self.sbLogoSizeY, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.editLogoPath.setEnabled( False)
      self.sbLogoSizeX.setEnabled( False)
      self.sbLogoSizeY.setEnabled( False)
      self.lLogo.addWidget( self.tLogoPath)
      self.lLogo.addWidget( self.editLogoPath)
      self.lLogo.addWidget( self.tLogoSize)
      self.lLogo.addWidget( self.sbLogoSizeX)
      self.lLogo.addWidget( self.sbLogoSizeY)
      self.lDrawing.addLayout( self.lLogo)

      self.mainLayout.addWidget( self.gDrawing)


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

      self.mainLayout.addWidget( self.gCorrectionSettings)


      self.gInputSettings = QtGui.QGroupBox('Input Sequence Pattern')
      self.lInputSettings = QtGui.QVBoxLayout()
      self.gInputSettings.setLayout( self.lInputSettings)

      self.editInputFile = QtGui.QLineEdit( self)
      QtCore.QObject.connect( self.editInputFile, QtCore.SIGNAL('textEdited(QString)'), self.inputFileChanged)
      self.lInputSettings.addWidget( self.editInputFile)

      self.lBrowseInput = QtGui.QHBoxLayout()
      self.lFilesCount = QtGui.QLabel('Files count:', self)
      self.editInputFilesCount = QtGui.QLineEdit( self)
      self.editInputFilesCount.setEnabled( False)
      self.lPattern = QtGui.QLabel('Pattern:', self)
      self.editInputFilesPattern = QtGui.QLineEdit( self)
      self.editInputFilesPattern.setEnabled( False)
      self.btnBrowseInput = QtGui.QPushButton('Browse sequence file', self)
      QtCore.QObject.connect( self.btnBrowseInput, QtCore.SIGNAL('pressed()'), self.browseInput)
      self.lBrowseInput.addWidget( self.lFilesCount)
      self.lBrowseInput.addWidget( self.editInputFilesCount)
      self.lBrowseInput.addWidget( self.lPattern)
      self.lBrowseInput.addWidget( self.editInputFilesPattern)
      self.lBrowseInput.addWidget( self.btnBrowseInput)
      self.lInputSettings.addLayout( self.lBrowseInput)

      self.lIdentify = QtGui.QHBoxLayout()
      self.tIdentify = QtGui.QLabel('Identify:', self)
      self.editIdentify = QtGui.QLineEdit( self)
      self.editIdentify.setEnabled( False)
      self.btnIdentify = QtGui.QPushButton('Refresh', self)
      QtCore.QObject.connect( self.btnIdentify, QtCore.SIGNAL('pressed()'), self.inputFileChanged)
      self.lIdentify.addWidget( self.tIdentify)
      self.lIdentify.addWidget( self.editIdentify)
      self.lIdentify.addWidget( self.btnIdentify)
      self.lInputSettings.addLayout( self.lIdentify)

      self.mainLayout.addWidget( self.gInputSettings)


      self.gOutputSettings = QtGui.QGroupBox('Output File')
      self.lOutputSettings = QtGui.QVBoxLayout()
      self.gOutputSettings.setLayout( self.lOutputSettings)

      self.lOutputName = QtGui.QHBoxLayout()
      self.tOutputName = QtGui.QLabel('Output file name:', self)
      self.editOutputName = QtGui.QLineEdit( self)
      QtCore.QObject.connect( self.editOutputName, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.btnBrowseOutput = QtGui.QPushButton('Browse', self)
      QtCore.QObject.connect( self.btnBrowseOutput, QtCore.SIGNAL('pressed()'), self.browseOutput)
      self.cAutoOutput = QtGui.QCheckBox('Auto', self)
      self.cAutoOutput.setChecked( True)
      QtCore.QObject.connect( self.cAutoOutput, QtCore.SIGNAL('stateChanged(int)'), self.autoOutput)
      self.cDateOutput = QtGui.QCheckBox('Add Date', self)
      self.cDateOutput.setChecked( True)
      QtCore.QObject.connect( self.cDateOutput, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.lOutputName.addWidget( self.tOutputName)
      self.lOutputName.addWidget( self.editOutputName)
      self.lOutputName.addWidget( self.btnBrowseOutput)
      self.lOutputName.addWidget( self.cAutoOutput)
      self.lOutputName.addWidget( self.cDateOutput)
      self.lOutputSettings.addLayout( self.lOutputName)

      self.lOutputDir = QtGui.QHBoxLayout()
      self.tOutputDir = QtGui.QLabel('Output folder:', self)
      self.editOutputDir = QtGui.QLineEdit( self)
      QtCore.QObject.connect( self.editOutputDir, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.lOutputDir.addWidget( self.tOutputDir)
      self.lOutputDir.addWidget( self.editOutputDir)
      self.lOutputSettings.addLayout( self.lOutputDir)

      self.mainLayout.addWidget( self.gOutputSettings)


      self.cmdField = QtGui.QTextEdit( self)
      self.cmdField.setReadOnly( True)
      self.mainLayout.addWidget( self.cmdField)

      self.lAfanasy = QtGui.QHBoxLayout()
      self.cAfanasy = QtGui.QCheckBox('Afanasy:', self)
      QtCore.QObject.connect( self.cAfanasy, QtCore.SIGNAL('stateChanged(int)'), self.afanasy)
      self.cAfOneTask = QtGui.QCheckBox('One Task', self)
      self.cAfOneTask.setChecked( True)
      self.cAfOneTask.setEnabled( False)
      QtCore.QObject.connect( self.cAfOneTask, QtCore.SIGNAL('stateChanged(int)'), self.afanasy)
      self.tAfCapacity = QtGui.QLabel('Capacity:', self)
      self.sbAfCapacity = QtGui.QSpinBox( self)
      self.sbAfCapacity.setRange( 1, 1000000)
      self.sbAfCapacity.setValue( 500)
      self.sbAfCapacity.setEnabled( False)
      QtCore.QObject.connect( self.sbAfCapacity, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.tAfCapConvert = QtGui.QLabel('Convert:', self)
      self.sbAfCapConvert = QtGui.QSpinBox( self)
      self.sbAfCapConvert.setRange( 1, 1000000)
      self.sbAfCapConvert.setValue( 10)
      self.sbAfCapConvert.setEnabled( False)
      QtCore.QObject.connect( self.sbAfCapConvert, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.tAfCapEncode = QtGui.QLabel('Encode:', self)
      self.sbAfCapEncode = QtGui.QSpinBox( self)
      self.sbAfCapEncode.setRange( 1, 1000000)
      self.sbAfCapEncode.setValue( 500)
      self.sbAfCapEncode.setEnabled( False)
      QtCore.QObject.connect( self.sbAfCapEncode, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.lAfanasy.addWidget( self.cAfanasy)
      self.lAfanasy.addWidget( self.cAfOneTask)
      self.lAfanasy.addWidget( self.tAfCapacity)
      self.lAfanasy.addWidget( self.sbAfCapacity)
      self.lAfanasy.addWidget( self.tAfCapConvert)
      self.lAfanasy.addWidget( self.sbAfCapConvert)
      self.lAfanasy.addWidget( self.tAfCapEncode)
      self.lAfanasy.addWidget( self.sbAfCapEncode)
      self.mainLayout.addLayout( self.lAfanasy)

      self.lProcess = QtGui.QHBoxLayout()
      self.btnRefresh = QtGui.QPushButton('Refresh', self)
      QtCore.QObject.connect( self.btnRefresh, QtCore.SIGNAL('pressed()'), self.evaluate)
      self.btnStart = QtGui.QPushButton('Start', self)
      self.btnStart.setEnabled( False)
      QtCore.QObject.connect( self.btnStart, QtCore.SIGNAL('pressed()'), self.execute)
      self.btnStop = QtGui.QPushButton('Stop', self)
      self.btnStop.setEnabled( False)
      QtCore.QObject.connect( self.btnStop, QtCore.SIGNAL('pressed()'), self.processStop)
      self.lProcess.addWidget( self.btnRefresh)
      self.lProcess.addWidget( self.btnStart)
      self.lProcess.addWidget( self.btnStop)
      self.mainLayout.addLayout( self.lProcess)

      self.autoTitle()
      self.autoOutput()
      self.inputFileChanged()
      self.evaluate()

   def autoOutput( self):
      enable = not self.cAutoOutput.isChecked()
      self.editOutputName.setEnabled( enable)
      self.btnBrowseOutput.setEnabled( enable)
      self.editOutputDir.setEnabled( enable)

   def autoTitle( self):
      enable = not self.cAutoTitle.isChecked()
      self.editTitle.setEnabled( enable)

   def drawLogo( self):
      enable = self.cLogo.isChecked()
      self.editLogoPath.setEnabled( enable)
      self.sbLogoSizeX.setEnabled( enable)
      self.sbLogoSizeY.setEnabled( enable)
      self.evaluate()

   def formatChanged( self):
      if self.cbFormat.itemData( self.cbFormat.currentIndex()).isNull():
         self.cLogo.setChecked( False)
         self.cLogo.setEnabled( False)
      else:
         self.cLogo.setEnabled( True)
      self.evaluate()

   def afanasy( self):
      enableAf = self.cAfanasy.isChecked()
      oneTask  = self.cAfOneTask.isChecked()
      self.cAfOneTask.setEnabled(     enableAf )
      self.sbAfCapacity.setEnabled(   enableAf and oneTask)
      self.sbAfCapConvert.setEnabled( enableAf and not oneTask)
      self.sbAfCapEncode.setEnabled(  enableAf and not oneTask)
      self.evaluate()

   def browseOutput( self):
      afile = QtGui.QFileDialog.getOpenFileName( self,'Choose a file')
      if afile.isEmpty(): return
      afile = str( afile)
      self.editOutputDir.setText( os.path.dirname( afile))
      afile = os.path.basename( afile)
      pos = afile.rfind('.')
      if pos > 0: afile = afile[ 0 : pos]
      self.editOutputName.setText( afile)
      self.evaluate()

   def browseInput( self):
      afile = QtGui.QFileDialog.getOpenFileName( self,'Choose a file')
      if afile.isEmpty(): return
      self.editInputFile.setText( afile)
      self.inputFileChanged()

   def inputFileChanged( self):
#      print 'inputFileChanged( self):'
      self.inputEvaluated = False
      self.editInputFilesCount.clear()
      self.editInputFilesPattern.clear()
      self.editIdentify.clear()
      inputfile = str( self.editInputFile.text())

      if len(inputfile) == 0:
         self.cmdField.setText('Choose one file from sequence')
         return

      pos = inputfile.rfind('file://')
      if pos >= 0:
         inputfile = inputfile[ pos+7 : len(inputfile)]
         self.editInputFile.setText( inputfile)

      inputfile = inputfile.strip()
      inputfile = inputfile.strip('\n')
      self.inputDir = os.path.dirname( inputfile)
      if not os.path.isdir( self.inputDir):
         self.cmdField.setText('Can\'t find input directory')
         return
      filename = os.path.basename( inputfile)
      digitsall = re.findall( r'(#{1,})', filename)
      if len(digitsall) == 0: digitsall = re.findall( r'([0-9]{1,})', filename)
      if len(digitsall):
         digits = digitsall[len(digitsall)-1]
         pos = filename.rfind(digits)
         self.inputPrefix = filename[0 : pos]
         self.inputPadding = len(digits)
         self.inputSuffix = filename[pos+self.inputPadding : len(filename)]
         padstr = ''
         for d in range(self.inputPadding): padstr += '#'
         pattern = self.inputPrefix + padstr + self.inputSuffix
      else:
         self.cmdField.setText('Can\'t find digits in input file name')
         return
      self.editInputFilesPattern.setText( pattern)
      self.inputPattern = os.path.join( self.inputDir, pattern)

      eprefix = re.escape( self.inputPrefix)
      esuffix = re.escape( self.inputSuffix)
      epad = self.inputPadding
      expr = re.compile( r'%(eprefix)s([0-9]{%(epad)d,%(epad)d})%(esuffix)s' % vars())
      filescount = 0
      allItems = os.listdir(self.inputDir)
      for item in allItems:
         if not os.path.isfile( os.path.join( self.inputDir, item)): continue
         if not expr.match( item): continue
         if filescount == 0: afile = item
         filescount += 1
      self.editInputFilesCount.setText(str(filescount))
      if filescount == 0:
         self.cmdField.setText('No files founded matching pattern.')
         return
      if filescount == 1:
         self.cmdField.setText('Founded only 1 file matching pattern.')
         return
      afile = os.path.join( self.inputDir, afile)
      pipe = subprocess.Popen( 'identify "%s"' % afile, shell=True, bufsize=100000, stdout=subprocess.PIPE).stdout
      output = pipe.read()
      if len(output) < len(afile):
         self.cmdField.setText('Invalid image.')
         return
      output = output.replace( afile, '')
      self.editIdentify.setText( output)

      self.inputEvaluated = True
      self.evaluate()

   def evaluate( self):
#      print 'evaluate( self):'
      self.evaluated = False
      if not self.inputEvaluated: return

      if self.cAutoTitle.isChecked(): self.editTitle.clear()
      if self.cAutoOutput.isChecked():
         self.editOutputName.clear()
         self.editOutputDir.clear()

      outdir = str( self.editOutputDir.text())
      if self.cAutoOutput.isChecked() or outdir == None or outdir == '':
         outdir = os.path.dirname( self.inputDir)
         self.editOutputDir.setText( outdir)

      outname = str( self.editOutputName.text())
      if self.cAutoOutput.isChecked() or outname == None or outname == '':
         outname = str( self.inputPrefix)
         outname = outname.strip('_. ')
         self.editOutputName.setText( outname)

      logodraw = self.cLogo.isChecked()
      logopath = self.editLogoPath.text()
      if logodraw:
         if not os.path.isfile( logopath):
            self.cmdField.setText('No logo file founded')
            return

      title = re.escape( str( self.editTitle.text()))
      if self.cAutoTitle.isChecked() or title == '':
         title = self.inputPrefix.strip('.')
         self.editTitle.setText( title)
      annotate = re.escape( str( self.editAnnotate.text()))

      cmd = 'mavishky.py'
      cmd = 'python ' + os.path.join( os.path.dirname( os.path.abspath( sys.argv[0])), cmd)
      format = self.cbFormat.itemData( self.cbFormat.currentIndex()).toString()
      if not format.isEmpty(): cmd += ' -r %s' % format
      cmd += ' -c %s' % self.cbCodec.itemData( self.cbCodec.currentIndex()).toString()
      cmd += ' -f %s' % self.cbFPS.currentText()
      cmd += ' -i "%s"' % self.inputPattern
      cmd += ' -o "%s"' % os.path.join( outdir, outname)
      cmd += ' -g %.2f' % self.dsbGamma.value()
      if self.cDateOutput.isChecked():
         cmd += ' -d'
      if self.cTitle.isChecked() and title != '':
         cmd += ' -t "%s"' % title
      if annotate != '':
         cmd += ' -a "%s"' % annotate
      if self.cFrame.isChecked():
         cmd += ' --drawframe'
      if self.cDate.isChecked():
         cmd += ' --drawdate'
      cacher = self.cbCacherH.itemData( self.cbCacherH.currentIndex()).toString()
      if not cacher.isEmpty(): cmd += ' --draw169 %s' % cacher
      cacher = self.cbCacherC.itemData( self.cbCacherC.currentIndex()).toString()
      if not cacher.isEmpty(): cmd += ' --draw235 %s' % cacher
      if self.cLogo.isChecked():
         cmd += ' --logopath "%s"' % logopath
         logosize  = str( self.sbLogoSizeX.value())
         logosize += 'x'
         logosize += str( self.sbLogoSizeY.value())
         cmd += ' --logosize %s' % logosize
      if self.cAfanasy.isChecked() and not self.cAfOneTask.isChecked():
         cmd += ' -A'
         cmd += ' --afconvcap %d' % self.sbAfCapConvert.value()
         cmd += ' --afenccap %d' % self.sbAfCapEncode.value()

      self.cmdField.setText( cmd)
      self.evaluated = True
      self.btnStart.setEnabled( True)

   def execute( self):
      if not self.evaluated: return
      self.command = str( self.cmdField.toPlainText())
      if len( self.command) == 0: return

      if self.cAfanasy.isChecked() and self.cAfOneTask.isChecked():
         af = __import__('af', globals(), locals(), [])
         job = af.Job( str(self.editOutputName.text()) + ' mavishka')
         block = af.Block('mavishky')
         block.setCapacity( self.sbAfCapacity.value())
         job.blocks.append( block)
         task = af.Task( str(self.editOutputName.text()))
         task.setCommand( self.command)
         block.tasks.append( task)
         job.send()
      else:
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
dialog = Dialog()
dialog.show()
app.exec_()
