#!/usr/bin/env python
# -*- coding: utf-8 -*-

from datetime import datetime
import os
import re
import subprocess
import sys

from PyQt4 import QtCore, QtGui

# Command arguments:

from optparse import OptionParser
Parser = OptionParser(usage="%prog [options]\ntype \"%prog -h\" for help", version="%prog 1.0")
Parser.add_option('-i', '--input',     dest='inputfile',    type  ='string',     default='',             help='Input file')
Parser.add_option('--company',         dest='company',      type  ='string',     default='Bazelevs VFX', help='Company name')
Parser.add_option('--project',         dest='project',      type  ='string',     default='',             help='Project name')
Parser.add_option('-D', '--debug',     dest='debug',        action='store_true', default=False,          help='Debug mode')

(Options, args) = Parser.parse_args()

# Initializations:

UserName = os.getenv('USER', os.getenv('USERNAME', 'user'))
# cut DOMAIN from username:
dpos = UserName.rfind('/')
if dpos == -1: dpos = UserName.rfind('\\')
if dpos != -1: UserName = UserName[dpos+1:]
UserName = UserName.capitalize()

DialogPath = os.path.dirname(os.path.abspath(sys.argv[0]))
LogosPath = os.path.join( DialogPath, 'logos')
FontsList = ['Arial','Courier-New','Impact','Tahoma','Times-New-Roman','Verdana']

class Dialog( QtGui.QWidget):
   def __init__( self):
      QtGui.QWidget.__init__( self)
      self.evaluated = False

      self.setWindowTitle('Mavishky !')
      self.mainLayout = QtGui.QVBoxLayout( self)

      self.lFormat = QtGui.QHBoxLayout()
      self.tFormat = QtGui.QLabel('Format:', self)
      self.cbFormat = QtGui.QComboBox( self)
      self.cbFormat.addItem('Encode "as is" only')
      self.cbFormat.addItem('PAL (720x576)', QtCore.QVariant('720x576'))
      self.cbFormat.addItem('PAL Square (768x576)', QtCore.QVariant('768x576'))
      self.cbFormat.addItem('HD 720p (1280x720)', QtCore.QVariant('1280x720'))
      self.cbFormat.addItem('HD 1080p (1920x1080)', QtCore.QVariant('1920x1080'))
      self.cbFormat.setCurrentIndex( 1)
      QtCore.QObject.connect( self.cbFormat, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.tCodec = QtGui.QLabel('Codec:', self)
      self.cbCodec = QtGui.QComboBox( self)
      self.cbCodec.addItem('Quicktime (PhotoJPG)', QtCore.QVariant('mjpeg'))
#      self.cbCodec.addItem('Quicktime (H.264)', QtCore.QVariant('h264'))
      self.cbCodec.addItem('Mpeg (MPEG4 XVID)', QtCore.QVariant('xvid'))
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

      self.lDraw = QtGui.QHBoxLayout()
      self.cFrame = QtGui.QCheckBox('Frame', self)
      self.cFrame.setChecked( True)
      self.lDraw.addWidget( self.cFrame)
      QtCore.QObject.connect( self.cFrame, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.cDate = QtGui.QCheckBox('Date', self)
      self.cDate.setChecked( True)
      self.lDraw.addWidget( self.cDate)
      QtCore.QObject.connect( self.cDate, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.cTime = QtGui.QCheckBox('Time', self)
      self.cTime.setChecked( False)
      self.lDraw.addWidget( self.cTime)
      QtCore.QObject.connect( self.cTime, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.cFileName = QtGui.QCheckBox('File Name', self)
      self.cFileName.setChecked( True)
      self.lDraw.addWidget( self.cFileName)
      QtCore.QObject.connect( self.cFileName, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.tCompany = QtGui.QLabel('Company:', self)
      self.lDraw.addWidget( self.tCompany)
      self.editCompany = QtGui.QLineEdit( Options.company, self)
      self.lDraw.addWidget( self.editCompany)
      QtCore.QObject.connect( self.editCompany, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.tFont = QtGui.QLabel('Font:', self)
      self.lDraw.addWidget( self.tFont)
      self.editFont = QtGui.QLineEdit('Arial', self)
      self.lDraw.addWidget( self.editFont)
      QtCore.QObject.connect( self.editFont, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.cbFont = QtGui.QComboBox( self)
      for font in FontsList: self.cbFont.addItem( font)
      self.lDraw.addWidget( self.cbFont)
      QtCore.QObject.connect( self.cbFont, QtCore.SIGNAL('currentIndexChanged(int)'), self.fontChanged)
      self.lDrawing.addLayout( self.lDraw)

      self.lTitles = QtGui.QHBoxLayout()
      self.cProject = QtGui.QCheckBox('Project:', self)
      self.cProject.setChecked( True)
      QtCore.QObject.connect( self.cProject, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.editProject = QtGui.QLineEdit( Options.project, self)
      QtCore.QObject.connect( self.editProject, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.cShot = QtGui.QCheckBox('Shot:', self)
      self.cShot.setChecked( True)
      QtCore.QObject.connect( self.cShot, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.editShot = QtGui.QLineEdit('', self)
      QtCore.QObject.connect( self.editShot, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.cVersion = QtGui.QCheckBox('Version:', self)
      self.cVersion.setChecked( True)
      QtCore.QObject.connect( self.cVersion, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.editVersion = QtGui.QLineEdit('', self)
      QtCore.QObject.connect( self.editVersion, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.cAutoTitles = QtGui.QCheckBox('Auto', self)
      self.cAutoTitles.setChecked( True)
      QtCore.QObject.connect( self.cAutoTitles, QtCore.SIGNAL('stateChanged(int)'), self.autoTitles)
      self.lTitles.addWidget( self.cProject)
      self.lTitles.addWidget( self.editProject)
      self.lTitles.addWidget( self.cShot)
      self.lTitles.addWidget( self.editShot)
      self.lTitles.addWidget( self.cVersion)
      self.lTitles.addWidget( self.editVersion)
      self.lTitles.addWidget( self.cAutoTitles)
      self.lDrawing.addLayout( self.lTitles)

      self.lUser = QtGui.QHBoxLayout()
      self.lArtist = QtGui.QLabel('Artist:', self)
      self.lUser.addWidget( self.lArtist)
      self.editArtist = QtGui.QLineEdit( UserName, self)
      self.lUser.addWidget( self.editArtist)
      QtCore.QObject.connect( self.editArtist, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.lActivity = QtGui.QLabel('Activity:', self)
      self.lUser.addWidget( self.lActivity)
      self.editActivity = QtGui.QLineEdit('', self)
      self.lUser.addWidget( self.editActivity)
      QtCore.QObject.connect( self.editActivity, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.cbActivity = QtGui.QComboBox( self)
      self.cbActivity.addItem('comp')
      self.cbActivity.addItem('render')
      self.cbActivity.addItem('anim')
      self.cbActivity.addItem('sim')
      self.lUser.addWidget( self.cbActivity)
      QtCore.QObject.connect( self.cbActivity, QtCore.SIGNAL('currentIndexChanged(int)'), self.activityChanged)
      self.lDrawing.addLayout( self.lUser)

      self.lComments = QtGui.QHBoxLayout()
      self.tComments = QtGui.QLabel('Comments:', self)
      self.editComments = QtGui.QLineEdit( self)
      QtCore.QObject.connect( self.editComments, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.lComments.addWidget( self.tComments)
      self.lComments.addWidget( self.editComments)
      self.lDrawing.addLayout( self.lComments)

      self.lLogo = QtGui.QHBoxLayout()
      self.cLogo = QtGui.QCheckBox('Logo', self)
      QtCore.QObject.connect( self.cLogo, QtCore.SIGNAL('stateChanged(int)'), self.drawLogo)
      self.tLogoPath = QtGui.QLabel('Path:', self)
      self.editLogoPath = QtGui.QLineEdit( os.path.join( LogosPath, 'logo.png'), self)
      QtCore.QObject.connect( self.editLogoPath, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.tLogoSize = QtGui.QLabel('Size:', self)
      self.sbLogoSizeX = QtGui.QSpinBox( self)
      self.sbLogoSizeX.setRange( 1, 1000)
      self.sbLogoSizeX.setValue( 200)
      QtCore.QObject.connect( self.sbLogoSizeX, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.sbLogoSizeY = QtGui.QSpinBox( self)
      self.sbLogoSizeY.setRange( 1, 1000)
      self.sbLogoSizeY.setValue( 100)
      QtCore.QObject.connect( self.sbLogoSizeY, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.btnBrowseLogo = QtGui.QPushButton('Browse', self)
      QtCore.QObject.connect( self.btnBrowseLogo, QtCore.SIGNAL('pressed()'), self.browseLogo)
      self.editLogoPath.setEnabled( False)
      self.btnBrowseLogo.setEnabled( False)
      self.sbLogoSizeX.setEnabled( False)
      self.sbLogoSizeY.setEnabled( False)
      self.lLogo.addWidget( self.cLogo)
      self.lLogo.addWidget( self.tLogoPath)
      self.lLogo.addWidget( self.editLogoPath)
      self.lLogo.addWidget( self.btnBrowseLogo)
      self.lLogo.addWidget( self.tLogoSize)
      self.lLogo.addWidget( self.sbLogoSizeX)
      self.lLogo.addWidget( self.sbLogoSizeY)
      self.lDrawing.addLayout( self.lLogo)

      self.lCacher = QtGui.QHBoxLayout()
      self.tCacherH = QtGui.QLabel('16:9 Cacher', self)
      self.cbCacherH = QtGui.QComboBox( self)
      self.cbCacherH.addItem('None')
      self.cbCacherH.addItem('25%', QtCore.QVariant('25'))
      self.cbCacherH.addItem('50%', QtCore.QVariant('50'))
      self.cbCacherH.addItem('75%', QtCore.QVariant('75'))
      self.cbCacherH.addItem('100%', QtCore.QVariant('100'))
      self.cbCacherH.setCurrentIndex( 1)
      QtCore.QObject.connect( self.cbCacherH, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.tCacherC = QtGui.QLabel('2.35 Cacher', self)
      self.cbCacherC = QtGui.QComboBox( self)
      self.cbCacherC.addItem('None')
      self.cbCacherC.addItem('25%', QtCore.QVariant('25'))
      self.cbCacherC.addItem('50%', QtCore.QVariant('50'))
      self.cbCacherC.addItem('75%', QtCore.QVariant('75'))
      self.cbCacherC.addItem('100%', QtCore.QVariant('100'))
      self.cbCacherC.setCurrentIndex( 1)
      QtCore.QObject.connect( self.cbCacherC, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.lCacher.addWidget( self.tCacherH)
      self.lCacher.addWidget( self.cbCacherH)
      self.lCacher.addWidget( self.tCacherC)
      self.lCacher.addWidget( self.cbCacherC)
      self.lDrawing.addLayout( self.lCacher)

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

      self.editInputFile = QtGui.QLineEdit( Options.inputfile, self)
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
      self.lOutputName.addWidget( self.tOutputName)
      self.editOutputName = QtGui.QLineEdit( self)
      self.lOutputName.addWidget( self.editOutputName)
      QtCore.QObject.connect( self.editOutputName, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.btnBrowseOutput = QtGui.QPushButton('Browse', self)
      self.lOutputName.addWidget( self.btnBrowseOutput)
      QtCore.QObject.connect( self.btnBrowseOutput, QtCore.SIGNAL('pressed()'), self.browseOutput)
      self.cAutoOutput = QtGui.QCheckBox('Auto', self)
      self.cAutoOutput.setChecked( True)
      self.lOutputName.addWidget( self.cAutoOutput)
      QtCore.QObject.connect( self.cAutoOutput, QtCore.SIGNAL('stateChanged(int)'), self.autoOutput)
      self.cDateOutput = QtGui.QCheckBox('Add Date', self)
      self.cDateOutput.setChecked( True)
      self.lOutputName.addWidget( self.cDateOutput)
      QtCore.QObject.connect( self.cDateOutput, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.cTimeOutput = QtGui.QCheckBox('Add Time', self)
      self.cTimeOutput.setChecked( False)
      self.lOutputName.addWidget( self.cTimeOutput)
      QtCore.QObject.connect( self.cTimeOutput, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
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

      self.inputEvaluated = False

      self.autoTitles()
      self.activityChanged()
      self.autoOutput()
      self.inputFileChanged()
      self.evaluate()

   def autoOutput( self):
      enable = not self.cAutoOutput.isChecked()
      self.editOutputName.setEnabled( enable)
      self.btnBrowseOutput.setEnabled( enable)
      self.editOutputDir.setEnabled( enable)

   def autoTitles( self):
      enable = not self.cAutoTitles.isChecked()
      self.editProject.setEnabled( enable)
      self.editShot.setEnabled( enable)
      self.editVersion.setEnabled( enable)

   def drawLogo( self):
      enable = self.cLogo.isChecked()
      self.btnBrowseLogo.setEnabled( enable)
      self.editLogoPath.setEnabled( enable)
      self.sbLogoSizeX.setEnabled( enable)
      self.sbLogoSizeY.setEnabled( enable)
      self.evaluate()

   def activityChanged( self):
      self.editActivity.setText( self.cbActivity.currentText())
      self.evaluate()

   def fontChanged( self):
      self.editFont.setText( self.cbFont.currentText())
      self.evaluate()

   def afanasy( self):
      enableAf = self.cAfanasy.isChecked()
      oneTask  = self.cAfOneTask.isChecked()
      self.cAfOneTask.setEnabled(     enableAf )
      self.sbAfCapacity.setEnabled(   enableAf and oneTask)
      self.sbAfCapConvert.setEnabled( enableAf and not oneTask)
      self.sbAfCapEncode.setEnabled(  enableAf and not oneTask)
      self.evaluate()

   def browseLogo( self):
      afile = QtGui.QFileDialog.getOpenFileName( self,'Choose a file', LogosPath)
      if afile.isEmpty(): return
      self.editLogoPath.setText( str( afile))
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
      afile = QtGui.QFileDialog.getOpenFileName( self,'Choose a file', self.editInputFile.text())
      if afile.isEmpty(): return
      self.editInputFile.setText( afile)
      self.inputFileChanged()

   def inputFileChanged( self):
      self.inputEvaluated = False
      self.editInputFilesCount.clear()
      self.editInputFilesPattern.clear()
      self.editIdentify.clear()
      inputfile = str( self.editInputFile.text())
      if sys.platform.find('win') == 0: inputfile = inputfile.replace('/','\\')

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
      if sys.platform.find('win') == 0: afile = afile.replace('/','\\')
      afile = os.path.join( self.inputDir, afile)
      pipe = subprocess.Popen( 'identify "%s"' % afile, shell=True, bufsize=100000, stdout=subprocess.PIPE).stdout
      output = pipe.read()
      if len(output) < len(afile):
         self.cmdField.setText('Invalid image.\n%s' % afile)
         return
      output = output.replace( afile, '')
      self.editIdentify.setText( output)

      self.inputEvaluated = True
      self.evaluate()

   def evaluate( self):
      self.evaluated = False
      if not self.inputEvaluated: return

      if self.cAutoTitles.isChecked(): self.editShot.clear()
      if self.cAutoOutput.isChecked():
         self.editOutputName.clear()
         self.editOutputDir.clear()

      project = re.escape( str( self.editProject.text()))
      if Options.project == '':
         if self.cAutoTitles.isChecked() or project == '':
            if sys.platform.find('win') == 0:
               pat_split = self.inputPattern.upper().split('\\')
               if len(pat_split) > 4: project = pat_split[4]
               else: project = pat_split[len(pat_split)-1]
            else:
               pat_split = self.inputPattern.upper().split('/')
               if len(pat_split) > 3: project = pat_split[3]
               else: project = pat_split[len(pat_split)-1]
            self.editProject.setText( project)

      shot = re.escape( str( self.editShot.text()))
      if self.cAutoTitles.isChecked() or shot == '':
         shot = self.inputPrefix.strip('.').upper()
         self.editShot.setText( shot)

      version = re.escape( str( self.editVersion.text()))
      if self.cAutoTitles.isChecked() or version == '':
         version = os.path.basename( os.path.dirname(self.inputPattern))
         self.editVersion.setText( version)

      company  = re.escape( str( self.editCompany.text()))
      artist   = re.escape( str( self.editArtist.text()))
      activity = re.escape( str( self.editActivity.text()))
      comments = re.escape( str( self.editComments.text()))

      outdir = str( self.editOutputDir.text())
      if self.cAutoOutput.isChecked() or outdir == None or outdir == '':
         outdir = os.path.dirname( self.inputDir)
         self.editOutputDir.setText( outdir)

      outname = str( self.editOutputName.text())
      if self.cAutoOutput.isChecked() or outname == None or outname == '':
         outname = shot
         if activity != '': outname += '_' + activity
         if version  != '': outname += '_' + version
         self.editOutputName.setText( outname)

      fontneeded = False
      if self.cProject.isChecked() and project != '': fontneeded = True
      if self.cShot.isChecked() and shot != '': fontneeded = True
      if comments != '': fontneeded = True
      if self.cFrame.isChecked(): fontneeded = True
      if self.cDate.isChecked(): fontneeded = True
      if self.cTime.isChecked(): fontneeded = True
      if fontneeded: fontpath = self.editFont.text()

      logodraw = self.cLogo.isChecked()
      logopath = self.editLogoPath.text()
      if logodraw:
         if not os.path.isfile( logopath):
            self.cmdField.setText('No logo file founded')
            return

      cmd = 'mavishky.py'
      cmd = 'python ' + os.path.join( os.path.dirname( os.path.abspath( sys.argv[0])), cmd)
      cmd += ' -c %s' % self.cbCodec.itemData( self.cbCodec.currentIndex()).toString()
      cmd += ' -f %s' % self.cbFPS.currentText()
      cmd += ' -i "%s"' % self.inputPattern
      cmd += ' -o "%s"' % os.path.join( outdir, outname)
      format = self.cbFormat.itemData( self.cbFormat.currentIndex()).toString()
      if not format.isEmpty():
         cmd += ' -r %s' % format
         cmd += ' -g %.2f' % self.dsbGamma.value()
         if self.cProject.isChecked() and project != '': cmd += ' --project "%s"' % project
         if self.cShot.isChecked() and shot != '': cmd += ' --shot "%s"' % shot
         if self.cVersion.isChecked() and version != '': cmd += ' --shotversion "%s"' % version
         if company != '': cmd += ' --company "%s"' % company
         if artist != '': cmd += ' --artist "%s"' % artist
         if activity != '': cmd += ' --activity "%s"' % activity
         if comments != '': cmd += ' --comments "%s"' % comments
         if self.cFrame.isChecked(): cmd += ' --drawframe'
         if self.cDate.isChecked(): cmd += ' --drawdate'
         if self.cTime.isChecked(): cmd += ' --drawtime'
         if self.cFileName.isChecked(): cmd += ' --drawfilename'
         if fontneeded: cmd += ' --font "%s"' % fontpath
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
      if self.cDateOutput.isChecked(): cmd += ' --datesuffix'
      if self.cTimeOutput.isChecked(): cmd += ' --timesuffix'
      if self.cAfanasy.isChecked() and not self.cAfOneTask.isChecked():
         cmd += ' -A'
         cmd += ' --afconvcap %d' % self.sbAfCapConvert.value()
         cmd += ' --afenccap %d' % self.sbAfCapEncode.value()
      if Options.debug: cmd += ' --debug'

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
