#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
import subprocess
import sys
import time

from PyQt4 import QtCore, QtGui

# Command arguments:

from optparse import OptionParser
Parser = OptionParser(usage="%prog [options] [file]\ntype \"%prog -h\" for help", version="%prog 1.0")
Parser.add_option('-s', '--slate',           dest='slate',           type  ='string',     default='dailies_slate',help='Slate frame template')
Parser.add_option('-t', '--template',        dest='template',        type  ='string',     default='dailies',      help='Frame paint template')
Parser.add_option('-f', '--format',          dest='format',          type  ='string',     default='720x576',      help='Resolution')
Parser.add_option('-c', '--codec',           dest='codec',           type  ='string',     default='photojpg_best.ffmpeg', help='Default codec preset')
Parser.add_option('--stereo',                dest='stereo',          action='store_true', default=False,          help='Stereo mode by default')
Parser.add_option('--fps',                   dest='fps',             type  ='string',     default='25',           help='Frames per second')
Parser.add_option('--company',               dest='company',         type  ='string',     default='Company',      help='Company name')
Parser.add_option('--project',               dest='project',         type  ='string',     default='',             help='Project name')
Parser.add_option('--suffix',                dest='suffix',          type  ='string',     default='',             help='Add suffix to output movie filename')
Parser.add_option('--draw169',               dest='draw169',         type  ='int',        default=25,             help='Draw 16:9 cacher opacity')
Parser.add_option('--draw235',               dest='draw235',         type  ='int',        default=25,             help='Draw 2.35 cacher opacity')
Parser.add_option('--line169',               dest='line169',         type  ='string',     default='200,200,200',  help='Draw 16:9 line color: "255,255,0"')
Parser.add_option('--line235',               dest='line235',         type  ='string',     default='200,200,200',  help='Draw 2.35 line color: "255,255,0"')
Parser.add_option('-A', '--afanasy',         dest='afanasy',         action='store_true', default=False,          help='Send Afanasy job')
Parser.add_option(      '--afpriority',      dest='afpriority',      type  ='int',        default=-1,             help='Afanasy job priority')
Parser.add_option(      '--afmaxhosts',      dest='afmaxhosts',      type  ='int',        default=-1,             help='Afanasy job maximum hosts')
Parser.add_option(      '--afhostsmask',     dest='afhostsmask',     type  ='string',     default='',             help='Afanasy job hosts mask')
Parser.add_option(      '--afhostsmaskex',   dest='afhostsmaskex',   type  ='string',     default='',             help='Afanasy job exclude hosts mask')
Parser.add_option(      '--afdependmask',    dest='afdependmask',    type  ='string',     default='',             help='Afanasy job depend mask')
Parser.add_option(      '--afdependmaskgl',  dest='afdependmaskgl',  type  ='string',     default='',             help='Afanasy job global depend mask')
Parser.add_option(      '--afcapacity',      dest='afcapacity',      type  ='int',        default=-1,             help='Afanasy job tasks capacity')
Parser.add_option(      '--afpause',         dest='afpause',         action='store_true', default=False,          help='Start Afanasy job paused')
Parser.add_option('-D', '--debug',           dest='debug',           action='store_true', default=False,          help='Debug mode')

(Options, args) = Parser.parse_args()

if len(args) > 2: parser.error('Too many arguments provided.')

InFile1 = ''
InFile2 = ''

if len(args) > 0: InFile1 = os.path.abspath( args[0])
if len(args) > 1: InFile2 = os.path.abspath( args[1])

# Initializations:
FormatNames  = ['Encode "as is" only', 'PAL (720x576)', 'PAL Square (768x576)', 'HD 720p (1280x720)', 'HD 1080p (1920x1080)']
FormatValues = [                   '',      '720x576' ,             '768x576' ,          '1280x720' ,           '1920x1080' ]

FPS = ['23.976','24','25','30']

UserName = os.getenv('USER', os.getenv('USERNAME', 'user'))
# cut DOMAIN from username:
dpos = UserName.rfind('/')
if dpos == -1: dpos = UserName.rfind('\\')
if dpos != -1: UserName = UserName[dpos+1:]
UserName = UserName.capitalize()

DialogPath = os.path.dirname(os.path.abspath(sys.argv[0]))
TemplatesPath = os.path.join( DialogPath, 'templates')
LogosPath = os.path.join( DialogPath, 'logos')
CodecsPath = os.path.join( DialogPath, 'codecs')

CacherNames  = ['None', '25%', '50%', '75%', '100%']
CacherValues = [   '0', '25' , '50' , '75' , '100' ]
FontsList = ['','Arial','Courier-New','Impact','Tahoma','Times-New-Roman','Verdana']
Encoders = ['ffmpeg', 'mencoder']
Gravity = ['SouthEast','South','SouthWest','West','NorthWest','North','NorthEast','East','Center']

# Process templates:
Templates = ['']
TemplateF = 0
TemplateS = 0
if os.path.isdir(TemplatesPath):
   files = os.listdir(TemplatesPath)
   files.sort()
   index = 0
   for afile in files:
      if afile[0] == '.': continue
      index += 1
      Templates.append(afile)
      if afile == Options.slate:    TemplateS = index
      if afile == Options.template: TemplateF = index


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

      self.setWindowTitle('Mavishky')
      self.mainLayout = QtGui.QVBoxLayout( self)
      self.tabwidget = QtGui.QTabWidget( self)
      self.mainLayout.addWidget( self.tabwidget)

      self.generalwidget = QtGui.QWidget( self)
      self.tabwidget.addTab( self.generalwidget,'General')
      self.generallayout = QtGui.QVBoxLayout( self.generalwidget)

      self.parameterswidget = QtGui.QWidget( self)
      self.tabwidget.addTab( self.parameterswidget,'Parameters')
      self.parameterslayout = QtGui.QVBoxLayout( self.parameterswidget)

      self.stereowidget = QtGui.QWidget( self)
      self.tabwidget.addTab( self.stereowidget,'Stereo')
      self.stereolayout = QtGui.QVBoxLayout( self.stereowidget)

      self.afanasywidget = QtGui.QWidget( self)
      self.tabwidget.addTab( self.afanasywidget,'Afanasy')
      self.afanasylayout = QtGui.QVBoxLayout( self.afanasywidget)


      # General:

      self.lFormat = QtGui.QHBoxLayout()
      self.tFormat = QtGui.QLabel('Format:', self)
      self.cbFormat = QtGui.QComboBox( self)
      i = 0
      for format in FormatValues:
         self.cbFormat.addItem( FormatNames[i], format)
         if format == Options.format: self.cbFormat.setCurrentIndex( i)
         i += 1
      QtCore.QObject.connect( self.cbFormat, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.tCodec = QtGui.QLabel('Codec:', self)
      self.cbCodec = QtGui.QComboBox( self)
      i = 0
      for name in CodecNames:
         self.cbCodec.addItem( name, QtCore.QVariant( CodecFiles[i]))
         if os.path.basename(CodecFiles[i]) == Options.codec: self.cbCodec.setCurrentIndex( i)
         i += 1
      QtCore.QObject.connect( self.cbCodec, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.tFPS = QtGui.QLabel('FPS:', self)
      self.cbFPS = QtGui.QComboBox( self)
      i = 0
      for fps in FPS:
         self.cbFPS.addItem(fps)
         if fps == Options.fps: self.cbFPS.setCurrentIndex( i)
         i += 1
      QtCore.QObject.connect( self.cbFPS, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.lFormat.addWidget( self.tFormat)
      self.lFormat.addWidget( self.cbFormat)
      self.lFormat.addWidget( self.tFPS)
      self.lFormat.addWidget( self.cbFPS)
      self.lFormat.addWidget( self.tCodec)
      self.lFormat.addWidget( self.cbCodec)
      self.generallayout.addLayout( self.lFormat)


      self.gInformation = QtGui.QGroupBox('Information')
      self.lInformation = QtGui.QVBoxLayout()
      self.gInformation.setLayout( self.lInformation)

      self.lTitles = QtGui.QHBoxLayout()
      self.tCompany = QtGui.QLabel('Company:', self)
      self.editCompany = QtGui.QLineEdit( Options.company, self)
      QtCore.QObject.connect( self.editCompany, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.tProject = QtGui.QLabel('Project:', self)
      self.editProject = QtGui.QLineEdit( Options.project, self)
      QtCore.QObject.connect( self.editProject, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.tShot = QtGui.QLabel('Shot:', self)
      self.editShot = QtGui.QLineEdit('', self)
      QtCore.QObject.connect( self.editShot, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.tVersion = QtGui.QLabel('Version:', self)
      self.editVersion = QtGui.QLineEdit('', self)
      QtCore.QObject.connect( self.editVersion, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.cAutoTitles = QtGui.QCheckBox('Auto', self)
      self.cAutoTitles.setChecked( True)
      QtCore.QObject.connect( self.cAutoTitles, QtCore.SIGNAL('stateChanged(int)'), self.autoTitles)
      self.lTitles.addWidget( self.tCompany)
      self.lTitles.addWidget( self.editCompany)
      self.lTitles.addWidget( self.tProject)
      self.lTitles.addWidget( self.editProject)
      self.lTitles.addWidget( self.tShot)
      self.lTitles.addWidget( self.editShot)
      self.lTitles.addWidget( self.tVersion)
      self.lTitles.addWidget( self.editVersion)
      self.lTitles.addWidget( self.cAutoTitles)
      self.lInformation.addLayout( self.lTitles)

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
      self.cbActivity.addItem('clnup')
      self.cbActivity.addItem('mtpnt')
      self.cbActivity.addItem('rnd')
      self.cbActivity.addItem('test')
      self.lUser.addWidget( self.cbActivity)
      QtCore.QObject.connect( self.cbActivity, QtCore.SIGNAL('currentIndexChanged(int)'), self.activityChanged)
      self.lInformation.addLayout( self.lUser)

      self.lComments = QtGui.QHBoxLayout()
      self.tComments = QtGui.QLabel('Comments:', self)
      self.editComments = QtGui.QLineEdit( self)
      QtCore.QObject.connect( self.editComments, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.lComments.addWidget( self.tComments)
      self.lComments.addWidget( self.editComments)
      self.lInformation.addLayout( self.lComments)

      self.generallayout.addWidget( self.gInformation)

      self.gInputSettings = QtGui.QGroupBox('Input Sequence Pattern')
      self.lInputSettings = QtGui.QVBoxLayout()
      self.gInputSettings.setLayout( self.lInputSettings)

      self.editInputFiles = QtGui.QLineEdit( InFile1, self)
      QtCore.QObject.connect( self.editInputFiles, QtCore.SIGNAL('textEdited(QString)'), self.inputFileChanged)
      self.lInputSettings.addWidget( self.editInputFiles)

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

      self.generallayout.addWidget( self.gInputSettings)


      self.gOutputSettings = QtGui.QGroupBox('Output File')
      self.lOutputSettings = QtGui.QVBoxLayout()
      self.gOutputSettings.setLayout( self.lOutputSettings)

      self.lOutputName = QtGui.QHBoxLayout()
      self.tOutputName = QtGui.QLabel('Output file name:', self)
      self.lOutputName.addWidget( self.tOutputName)
      self.editOutputName = QtGui.QLineEdit( self)
      self.lOutputName.addWidget( self.editOutputName)
      QtCore.QObject.connect( self.editOutputName, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.cAutoOutputName = QtGui.QCheckBox('Auto', self)
      self.cAutoOutputName.setChecked( True)
      self.lOutputName.addWidget( self.cAutoOutputName)
      QtCore.QObject.connect( self.cAutoOutputName, QtCore.SIGNAL('stateChanged(int)'), self.autoOutputName)
      self.tOutputSuffix = QtGui.QLabel('Suffix:')
      self.lOutputName.addWidget( self.tOutputSuffix)
      self.editOutputSuffix = QtGui.QLineEdit( Options.suffix, self)
      self.lOutputName.addWidget( self.editOutputSuffix)
      QtCore.QObject.connect( self.editOutputSuffix, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.editOutputSuffix.setMaximumWidth(60)
      self.lOutputSettings.addLayout( self.lOutputName)

      self.lOutputDir = QtGui.QHBoxLayout()
      self.tOutputDir = QtGui.QLabel('Output folder:', self)
      self.lOutputDir.addWidget( self.tOutputDir)
      self.editOutputDir = QtGui.QLineEdit( self)
      QtCore.QObject.connect( self.editOutputDir, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.lOutputDir.addWidget( self.editOutputDir)
      self.btnBrowseOutputDir = QtGui.QPushButton('Browse', self)
      QtCore.QObject.connect( self.btnBrowseOutputDir, QtCore.SIGNAL('pressed()'), self.browseOutputFolder)
      self.lOutputDir.addWidget( self.btnBrowseOutputDir)
      self.lOutputSettings.addLayout( self.lOutputDir)

      self.generallayout.addWidget( self.gOutputSettings)


      # Parameters:

      self.lTemplates = QtGui.QHBoxLayout()
      self.tTemplateS = QtGui.QLabel('Slate Template:', self)
      self.tTemplateF = QtGui.QLabel('Frame Template:', self)
      self.cbTemplateS = QtGui.QComboBox( self)
      self.cbTemplateF = QtGui.QComboBox( self)
      for template in Templates:
         self.cbTemplateS.addItem(template)
         self.cbTemplateF.addItem(template)
      self.cbTemplateS.setCurrentIndex( TemplateS)
      self.cbTemplateF.setCurrentIndex( TemplateF)
      QtCore.QObject.connect( self.cbTemplateS, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      QtCore.QObject.connect( self.cbTemplateF, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.lTemplates.addWidget( self.tTemplateS)
      self.lTemplates.addWidget( self.cbTemplateS)
      self.lTemplates.addWidget( self.tTemplateF)
      self.lTemplates.addWidget( self.cbTemplateF)
      self.parameterslayout.addLayout( self.lTemplates)

      self.gDrawing = QtGui.QGroupBox('Drawing')
      self.lDrawing = QtGui.QVBoxLayout()
      self.gDrawing.setLayout( self.lDrawing)

      self.cTime = QtGui.QCheckBox('Add Time To Date', self)
      self.cTime.setChecked( False)
      QtCore.QObject.connect( self.cTime, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.lDrawing.addWidget( self.cTime)

      self.lCacher = QtGui.QHBoxLayout()
      self.tCacher169 = QtGui.QLabel('16:9 Cacher:', self)
      self.cbCacher169 = QtGui.QComboBox( self)
      i = 0
      for cacher in CacherNames:
         self.cbCacher169.addItem( cacher, QtCore.QVariant( CacherValues[i]))
         if CacherValues[i] == str(Options.draw169): self.cbCacher169.setCurrentIndex( i)
         i += 1
      QtCore.QObject.connect( self.cbCacher169, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.tCacher235 = QtGui.QLabel('2.35 Cacher:', self)
      self.cbCacher235 = QtGui.QComboBox( self)
      i = 0
      for cacher in CacherNames:
         self.cbCacher235.addItem( cacher, QtCore.QVariant( CacherValues[i]))
         if CacherValues[i] == str(Options.draw235): self.cbCacher235.setCurrentIndex( i)
         i += 1
      QtCore.QObject.connect( self.cbCacher235, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.lCacher.addWidget( self.tCacher169)
      self.lCacher.addWidget( self.cbCacher169)
      self.lCacher.addWidget( self.tCacher235)
      self.lCacher.addWidget( self.cbCacher235)
      self.lDrawing.addLayout( self.lCacher)

      self.lLines = QtGui.QHBoxLayout()
      self.tLine169 = QtGui.QLabel('Line 16:9 Color:', self)
      self.lLines.addWidget( self.tLine169)
      self.editLine169 = QtGui.QLineEdit( Options.line169, self)
      self.lLines.addWidget( self.editLine169)
      QtCore.QObject.connect( self.editLine169, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.tLine235 = QtGui.QLabel('Line 2.35 Color:', self)
      self.lLines.addWidget( self.tLine235)
      self.editLine235 = QtGui.QLineEdit( Options.line235, self)
      self.lLines.addWidget( self.editLine235)
      QtCore.QObject.connect( self.editLine235, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.lDrawing.addLayout( self.lLines)

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
      self.tLogoGravity = QtGui.QLabel('Position:', self)
      self.cbLogoGravity = QtGui.QComboBox( self)
      for pos in Gravity: self.cbLogoGravity.addItem( pos)
      QtCore.QObject.connect( self.cbLogoGravity, QtCore.SIGNAL('currentIndexChanged(int)'), self.evaluate)
      self.editLogoPath.setEnabled( False)
      self.btnBrowseLogo.setEnabled( False)
      self.sbLogoSizeX.setEnabled( False)
      self.sbLogoSizeY.setEnabled( False)
      self.cbLogoGravity.setEnabled( False)
      self.lLogo.addWidget( self.cLogo)
      self.lLogo.addWidget( self.tLogoPath)
      self.lLogo.addWidget( self.editLogoPath)
      self.lLogo.addWidget( self.btnBrowseLogo)
      self.lLogo.addWidget( self.tLogoSize)
      self.lLogo.addWidget( self.sbLogoSizeX)
      self.lLogo.addWidget( self.sbLogoSizeY)
      self.lLogo.addWidget( self.tLogoGravity)
      self.lLogo.addWidget( self.cbLogoGravity)
      self.lDrawing.addLayout( self.lLogo)

      self.lFont = QtGui.QHBoxLayout()
      self.tFont = QtGui.QLabel('Annotations Text Font:', self)
      self.lFont.addWidget( self.tFont)
      self.editFont = QtGui.QLineEdit('', self)
      self.lFont.addWidget( self.editFont)
      QtCore.QObject.connect( self.editFont, QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.cbFont = QtGui.QComboBox( self)
      for font in FontsList: self.cbFont.addItem( font)
      self.lFont.addWidget( self.cbFont)
      QtCore.QObject.connect( self.cbFont, QtCore.SIGNAL('currentIndexChanged(int)'), self.fontChanged)
      self.lDrawing.addLayout( self.lFont)

      self.parameterslayout.addWidget( self.gDrawing)

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

      self.dateTimeLayout = QtGui.QHBoxLayout()
      self.cDateOutput = QtGui.QCheckBox('Append Name With Date', self)
      self.cDateOutput.setChecked( False)
      self.dateTimeLayout.addWidget( self.cDateOutput)
      QtCore.QObject.connect( self.cDateOutput, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.cTimeOutput = QtGui.QCheckBox('Append Name With Time', self)
      self.cTimeOutput.setChecked( False)
      self.dateTimeLayout.addWidget( self.cTimeOutput)
      self.parameterslayout.addLayout( self.dateTimeLayout)
      QtCore.QObject.connect( self.cTimeOutput, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)


      # Stereo:

      self.cStereoDuplicate = QtGui.QCheckBox('Duplicate first sequence', self)
      self.cStereoDuplicate.setChecked( Options.stereo)
      QtCore.QObject.connect( self.cStereoDuplicate, QtCore.SIGNAL('stateChanged(int)'), self.evalStereo)
      self.stereolayout.addWidget( self.cStereoDuplicate)

      # Second Pattern:
      self.gInputFileGroup2 = QtGui.QGroupBox('Second Sequence Pattern')
      self.stereolayout.addWidget( self.gInputFileGroup2)
      self.lInputFileGroup2 = QtGui.QVBoxLayout()
      self.gInputFileGroup2.setLayout( self.lInputFileGroup2)

      self.editInputFiles2 = QtGui.QLineEdit( InFile2, self)
      self.lInputFileGroup2.addWidget( self.editInputFiles2)
      QtCore.QObject.connect( self.editInputFiles2, QtCore.SIGNAL('textEdited(QString)'), self.inputFileChanged2)

      self.leditInputFileCtrl2 = QtGui.QHBoxLayout()
      self.btnInputFileCopy = QtGui.QPushButton('Copy&&Paste First Sequence', self)
      self.leditInputFileCtrl2.addWidget( self.btnInputFileCopy)
      QtCore.QObject.connect( self.btnInputFileCopy, QtCore.SIGNAL('pressed()'), self.copyInput)
      self.tInputFilesCount2 = QtGui.QLabel('Files count:', self)
      self.leditInputFileCtrl2.addWidget( self.tInputFilesCount2)
      self.editInputFilesCount2 = QtGui.QLineEdit( self)
      self.leditInputFileCtrl2.addWidget( self.editInputFilesCount2)
      self.editInputFilesCount2.setEnabled( False)
      self.tInputFilesPattern2 = QtGui.QLabel('Pattern:', self)
      self.leditInputFileCtrl2.addWidget( self.tInputFilesPattern2)
      self.editInputFilesPattern2 = QtGui.QLineEdit( self)
      self.leditInputFileCtrl2.addWidget( self.editInputFilesPattern2)
      self.editInputFilesPattern2.setEnabled( False)
      self.btnInputFileBrowse2 = QtGui.QPushButton('Browse', self)
      self.leditInputFileCtrl2.addWidget( self.btnInputFileBrowse2)
      QtCore.QObject.connect( self.btnInputFileBrowse2, QtCore.SIGNAL('pressed()'), self.browseInput2)
      self.lInputFileGroup2.addLayout( self.leditInputFileCtrl2)

      self.lIdentify2 = QtGui.QHBoxLayout()
      self.tIdentify2 = QtGui.QLabel('Identify:', self)
      self.lIdentify2.addWidget( self.tIdentify2)
      self.editIdentify2 = QtGui.QLineEdit( self)
      self.editIdentify2.setEnabled( False)
      self.lIdentify2.addWidget( self.editIdentify2)
      self.btnInputFileRefresh2 = QtGui.QPushButton('Refresh', self)
      self.lIdentify2.addWidget( self.btnInputFileRefresh2)
      QtCore.QObject.connect( self.btnInputFileRefresh2, QtCore.SIGNAL('pressed()'), self.inputFileChanged2)
      self.lInputFileGroup2.addLayout( self.lIdentify2)

      self.lStereoStatus = QtGui.QHBoxLayout()
      self.stereolayout.addLayout( self.lStereoStatus)
      self.tStereoStatus = QtGui.QLabel( 'Stereo Status:', self)
      self.tStereoStatus.setAutoFillBackground( True)
      self.lStereoStatus.addWidget( self.tStereoStatus)
      self.editStereoStatus = QtGui.QLineEdit( self)
      self.lStereoStatus.addWidget( self.editStereoStatus)
      self.editStereoStatus.setReadOnly( True)

      # Afanasy:

      self.cAfanasy = QtGui.QCheckBox('Enable', self)
      self.cAfanasy.setChecked( Options.afanasy)
      QtCore.QObject.connect( self.cAfanasy, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.afanasylayout.addWidget( self.cAfanasy)

      # Priority
      self.lAfPriority = QtGui.QHBoxLayout()
      self.tAfPriority = QtGui.QLabel('Priority:', self)
      self.lAfPriority.addWidget( self.tAfPriority)
      self.sbAfPriority = QtGui.QSpinBox( self)
      self.sbAfPriority.setRange( -1, 1000000)
      self.sbAfPriority.setValue( Options.afpriority)
      QtCore.QObject.connect( self.sbAfPriority, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.lAfPriority.addWidget( self.sbAfPriority)
      self.tAfPriorityDef = QtGui.QLabel('"-1" Means default value.', self)
      self.lAfPriority.addWidget( self.tAfPriorityDef)
      self.afanasylayout.addLayout( self.lAfPriority)

      # Hosts
      self.gAfHosts = QtGui.QGroupBox('Hosts')
      self.afanasylayout.addWidget( self.gAfHosts)
      self.lAfHosts = QtGui.QVBoxLayout()
      self.gAfHosts.setLayout( self.lAfHosts)

      self.lAfMaxHosts = QtGui.QHBoxLayout()
      self.lAfHosts.addLayout( self.lAfMaxHosts)
      self.tAfMaxHosts = QtGui.QLabel('Maximum Number:', self)
      self.lAfMaxHosts.addWidget( self.tAfMaxHosts)
      self.sbAfMaxHosts = QtGui.QSpinBox( self)
      self.sbAfMaxHosts.setRange( -1, 1000000)
      self.sbAfMaxHosts.setValue( Options.afmaxhosts)
      QtCore.QObject.connect( self.sbAfMaxHosts, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.lAfMaxHosts.addWidget( self.sbAfMaxHosts)
      self.tAfMaxHostsDef = QtGui.QLabel('"-1" Means no hosts count limit.', self)
      self.lAfMaxHosts.addWidget( self.tAfMaxHostsDef)

      self.lAfHostsMask = QtGui.QHBoxLayout()
      self.lAfHosts.addLayout( self.lAfHostsMask)
      self.tAfHostsMask = QtGui.QLabel('Hosts Names Mask:', self)
      self.lAfHostsMask.addWidget( self.tAfHostsMask)
      self.editAfHostsMask = QtGui.QLineEdit( Options.afhostsmask, self)
      QtCore.QObject.connect( self.editAfHostsMask, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.lAfHostsMask.addWidget( self.editAfHostsMask)
      self.tAfHostsMaskDef = QtGui.QLabel('Leave empty to run on any host.', self)
      self.lAfHostsMask.addWidget( self.tAfHostsMaskDef)

      self.lAfHostsMaskExclude = QtGui.QHBoxLayout()
      self.lAfHosts.addLayout( self.lAfHostsMaskExclude)
      self.tAfHostsMaskExclude = QtGui.QLabel('Exclude Hosts Names Mask:', self)
      self.lAfHostsMaskExclude.addWidget( self.tAfHostsMaskExclude)
      self.editAfHostsMaskExclude = QtGui.QLineEdit( Options.afhostsmaskex, self)
      QtCore.QObject.connect( self.editAfHostsMaskExclude, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.lAfHostsMaskExclude.addWidget( self.editAfHostsMaskExclude)
      self.tAfHostsMaskExcludeDef = QtGui.QLabel('Leave empty not to exclude any host.', self)
      self.lAfHostsMaskExclude.addWidget( self.tAfHostsMaskExcludeDef)

      # Depends
      self.gAfDepends = QtGui.QGroupBox('Depends')
      self.afanasylayout.addWidget( self.gAfDepends)
      self.lAfDepends = QtGui.QVBoxLayout()
      self.gAfDepends.setLayout( self.lAfDepends)

      self.lAfDependMask = QtGui.QHBoxLayout()
      self.lAfDepends.addLayout( self.lAfDependMask)
      self.tAfDependMask = QtGui.QLabel('Depend Jobs Mask:', self)
      self.lAfDependMask.addWidget( self.tAfDependMask)
      self.editAfDependMask = QtGui.QLineEdit( Options.afdependmask, self)
      QtCore.QObject.connect( self.editAfDependMask, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.lAfDependMask.addWidget( self.editAfDependMask)
      self.tAfDependMaskDef = QtGui.QLabel('Leave empty not to wait any jobs.', self)
      self.lAfDependMask.addWidget( self.tAfDependMaskDef)

      self.lAfDependMaskGlobal = QtGui.QHBoxLayout()
      self.lAfDepends.addLayout( self.lAfDependMaskGlobal)
      self.tAfDependMaskGlobal = QtGui.QLabel('Global Depend Jobs Mask:', self)
      self.lAfDependMaskGlobal.addWidget( self.tAfDependMaskGlobal)
      self.editAfDependMaskGlobal = QtGui.QLineEdit( Options.afdependmaskgl, self)
      QtCore.QObject.connect( self.editAfDependMaskGlobal, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)
      self.lAfDependMaskGlobal.addWidget( self.editAfDependMaskGlobal)
      self.tAfDependMaskGlobalDef = QtGui.QLabel('Set mask to wait any user jobs.', self)
      self.lAfDependMaskGlobal.addWidget( self.tAfDependMaskGlobalDef)

      # Capacity
      self.gAfCapacity = QtGui.QGroupBox('Capacity')
      self.lAfCapacity = QtGui.QHBoxLayout()
      self.gAfCapacity.setLayout( self.lAfCapacity)

      self.cAfOneTask = QtGui.QCheckBox('One Task', self)
      self.cAfOneTask.setChecked( True)
      QtCore.QObject.connect( self.cAfOneTask, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)

      self.tAfCapacity = QtGui.QLabel('Capacity:', self)
      self.sbAfCapacity = QtGui.QSpinBox( self)
      self.sbAfCapacity.setRange( -1, 1000000)
      self.sbAfCapacity.setValue( Options.afcapacity)
      QtCore.QObject.connect( self.sbAfCapacity, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)

      self.tAfCapConvert = QtGui.QLabel('Convert:', self)
      self.sbAfCapConvert = QtGui.QSpinBox( self)
      self.sbAfCapConvert.setRange( -1, 1000000)
      self.sbAfCapConvert.setValue( Options.afcapacity)
      QtCore.QObject.connect( self.sbAfCapConvert, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)

      self.tAfCapEncode = QtGui.QLabel('Encode:', self)
      self.sbAfCapEncode = QtGui.QSpinBox( self)
      self.sbAfCapEncode.setRange( -1, 1000000)
      self.sbAfCapEncode.setValue( Options.afcapacity)
      QtCore.QObject.connect( self.sbAfCapEncode, QtCore.SIGNAL('valueChanged(int)'), self.evaluate)

      self.lAfCapacity.addWidget( self.cAfOneTask)
      self.lAfCapacity.addWidget( self.tAfCapacity)
      self.lAfCapacity.addWidget( self.sbAfCapacity)
      self.lAfCapacity.addWidget( self.tAfCapConvert)
      self.lAfCapacity.addWidget( self.sbAfCapConvert)
      self.lAfCapacity.addWidget( self.tAfCapEncode)
      self.lAfCapacity.addWidget( self.sbAfCapEncode)
      self.afanasylayout.addWidget( self.gAfCapacity)

      # Pause
      self.lAfPause = QtGui.QHBoxLayout()
      self.afanasylayout.addLayout( self.lAfPause)

      self.cAfPause = QtGui.QCheckBox('Start Job Paused', self)
      self.cAfPause.setChecked( Options.afpause)
      QtCore.QObject.connect( self.cAfPause, QtCore.SIGNAL('stateChanged(int)'), self.evaluate)
      self.lAfPause.addWidget( self.cAfPause)

      self.tAfTime = QtGui.QLabel('Start At Time:', self)
      self.lAfPause.addWidget( self.tAfTime)
      self.editAfTime = QtGui.QDateTimeEdit( QtCore.QDateTime.currentDateTime(), self)
      self.editAfTime.setDisplayFormat('dddd d MMMM yyyy, h:mm')
      QtCore.QObject.connect( self.editAfTime, QtCore.SIGNAL('dateTimeChanged()'), self.evaluate)
      self.lAfPause.addWidget( self.editAfTime)


      # Output Field:

      self.cmdField = QtGui.QTextEdit( self)
      self.cmdField.setReadOnly( True)
      self.mainLayout.addWidget( self.cmdField)


      # Main Buttons:

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


      self.inputPattern = None
      self.inputPattern2 = None
      self.autoTitles()
      self.activityChanged()
      self.autoOutputName()
      self.inputFileChanged()
      self.inputFileChanged2()
      self.evaluate()

   def evalStereo( self):
      if self.inputPattern2 is None:
         self.cStereoDuplicate.setEnabled( True)
         if self.cStereoDuplicate.isChecked():
            self.editStereoStatus.setText('Stereo from one sequence.')
            self.tStereoStatus.setBackgroundRole( QtGui.QPalette.Dark)
         else:
            self.editStereoStatus.setText('No stereo. Specify second sequence or enable duplicate one sequence.')
            self.tStereoStatus.setAutoFillBackground( True)
            self.tStereoStatus.setBackgroundRole( QtGui.QPalette.Window)
      else:
         self.cStereoDuplicate.setChecked( False)
         self.cStereoDuplicate.setEnabled( False)
         if self.editInputFilesCount.text() == self.editInputFilesCount2.text():
            self.editStereoStatus.setText('Stereo from two sequences.')
            self.tStereoStatus.setBackgroundRole( QtGui.QPalette.LinkVisited)
         else:
            self.inputPattern2 = None
            self.editStereoStatus.setText('Two sequences must be the same length.')
            self.evaluated = False
            self.btnStart.setEnabled( False)
            self.cmdField.setText('Sequences length mismatch.')
            return
      if self.inputPattern is not None:
         self.evaluate()

   def copyInput( self):
      files1 = self.editInputFiles.text()
      if not files1.isEmpty():
         self.editInputFiles2.setText( files1)
      self.inputFileChanged2()

   def autoOutputName( self):
      enable = not self.cAutoOutputName.isChecked()
      self.editOutputName.setEnabled( enable)
      self.editOutputSuffix.setEnabled( not enable)

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
      self.cbLogoGravity.setEnabled(enable)
      self.evaluate()

   def activityChanged( self):
      self.editActivity.setText( self.cbActivity.currentText())
      self.evaluate()

   def fontChanged( self):
      self.editFont.setText( self.cbFont.currentText())
      self.evaluate()

   def browseLogo( self):
      afile = QtGui.QFileDialog.getOpenFileName( self,'Choose a file', LogosPath)
      if afile.isEmpty(): return
      self.editLogoPath.setText( '%s' % afile)
      self.evaluate()

   def browseOutputFolder( self):
      folder = QtGui.QFileDialog.getExistingDirectory( self,'Choose a directory', os.path.dirname('%s' % self.editOutputDir.text()))
      if not folder.isEmpty(): self.editOutputDir.setText( folder)

   def browseInput( self):
      afile = QtGui.QFileDialog.getOpenFileName( self,'Choose a file', self.editInputFiles.text())
      if afile.isEmpty(): return
      self.editInputFiles.setText( afile)
      self.inputFileChanged()

   def browseInput2( self):
      afile = QtGui.QFileDialog.getOpenFileName( self,'Choose a file', self.editInputFiles2.text())
      if afile.isEmpty(): return
      self.editInputFiles2.setText( afile)
      self.inputFileChanged2()

   def inputFileChanged( self):
      self.editInputFilesCount.clear()
      self.editInputFilesPattern.clear()
      self.editIdentify.clear()
      inputfile = '%s' % self.editInputFiles.text()
      InputFile, InputPattern, FilesCount, Identify = self.calcPattern( inputfile)

      self.inputPattern = InputPattern
      if InputPattern == None: return

      self.editInputFiles.setText( InputFile)
      self.editInputFilesPattern.setText( os.path.basename( InputPattern))
      self.editInputFilesCount.setText( '%d' % FilesCount)
      self.editIdentify.setText( Identify)

      self.evaluate()

   def inputFileChanged2( self):
      self.editInputFilesCount2.clear()
      self.editInputFilesPattern2.clear()
      self.editIdentify2.clear()
      inputfile = '%s' % self.editInputFiles2.text()
      InputFile, InputPattern, FilesCount, Identify = self.calcPattern( inputfile)

      self.inputPattern2 = InputPattern
      if InputPattern is not None:
         self.editInputFiles2.setText( InputFile)
         self.editInputFilesPattern2.setText( os.path.basename( InputPattern))
         self.editInputFilesCount2.setText( '%d' % FilesCount)
         self.editIdentify2.setText( Identify)
      self.evalStereo()

   def calcPattern( self, InputFile):
      self.evaluated = False
      self.btnStart.setEnabled( False)

      InputPattern = None
      FilesCount = 0
      Identify = ''
      if sys.platform.find('win') == 0: InputFile = InputFile.replace('/','\\')

      if len(InputFile) == 0:
         self.cmdField.setText('Choose one file from sequence.')
         return InputFile, InputPattern, FilesCount, Identify

      # Remove link and strip filename:
      pos = InputFile.rfind('file://')
      if pos >= 0: InputFile = InputFile[ pos+7 : ]
      InputFile = InputFile.strip()
      InputFile = InputFile.strip('\n')

      # If directory is specified, use the first file in it:
      if os.path.isdir( InputFile):
         dirfiles = os.listdir( InputFile)
         if len( dirfiles) == 0:
            print 'Folder "%s" is empty.' % InputFile
            return InputFile, InputPattern, FilesCount, Identify
         InputFile = os.path.join( InputFile, dirfiles[0])

      inputdir = os.path.dirname( InputFile)
      if not os.path.isdir( inputdir):
         self.cmdField.setText('Can\'t find input directory.')
         return InputFile, InputPattern, FilesCount, Identify
      filename = os.path.basename( InputFile)

      # Search %04d pattern:
      digitsall = re.findall(r'%0\dd', filename)
      if len(digitsall):
         padstr = digitsall[-1]
         padding = int( padstr[2])
         pos = filename.rfind( padstr)
         prefix = filename[ : pos]
         suffix = filename[pos+4 : ]
      else:
         # Search #### pattern:
         digitsall = re.findall(r'(#{1,})', filename)
         if len(digitsall) == 0:
            # Search digits pattern:
            digitsall = re.findall(r'([0-9]{1,})', filename)
         if len(digitsall):
            digits = digitsall[-1]
            pos = filename.rfind(digits)
            prefix = filename[ : pos]
            padding = len(digits)
            suffix = filename[pos+padding : ]
            padstr = ''
            for d in range(padding): padstr += '#'
         else:
            self.cmdField.setText('Can\'t find digits in input file name.')
            return InputFile, InputPattern, FilesCount, Identify

      pattern = prefix + padstr + suffix

      expr = re.compile( r'%(prefix)s([0-9]{%(padding)d,%(padding)d})%(suffix)s' % vars())
      FilesCount = 0
      allItems = os.listdir( inputdir)
      for item in allItems:
         if not os.path.isfile( os.path.join( inputdir, item)): continue
         match = expr.match( item)
         if not match: continue
         if match.group(0) != item: continue
         if FilesCount == 0: afile = item
         FilesCount += 1
      if FilesCount <= 1:
         self.cmdField.setText('None or only one file founded matching pattern.\n\
         prefix, padding, suffix = "%(prefix)s" %(padding)d "%(suffix)s"' % vars())
         return InputFile, InputPattern, FilesCount, Identify
      if sys.platform.find('win') == 0: afile = afile.replace('/','\\')
      afile = os.path.join( inputdir, afile)
      identify = 'convert -identify "%s"'
      if sys.platform.find('win') == 0: identify += ' nul'
      else: identify += ' /dev/null'
      pipe = subprocess.Popen( identify % afile, shell=True, bufsize=100000, stdout=subprocess.PIPE).stdout
      Identify = pipe.read()
      if len(Identify) < len(afile):
         self.cmdField.setText('Invalid image.\n%s' % afile)
         return InputFile, InputPattern, FilesCount, Identify
      Identify = Identify.replace( afile, '')

      InputPattern = os.path.join( inputdir, pattern)

      return InputFile, InputPattern, FilesCount, Identify

   def validateEditColor( self, string, message):
      if string is None: return False
      if string == '': return True
      values = string.split(',')
      if len( values) == 3:
         passed = True
         for value in values:
            if len( value) < 1 or len(value) > 3:
               passed = False
               break
            for digit in value:
               if not digit in '1234567890':
                  passed = False
                  break
         if passed: return True
      self.cmdField.setText('Invalid %s color string. Example: "255,255,0" - yellow.' % message)
      return False


   def evaluate( self):
      self.evaluated = False
      self.btnStart.setEnabled( False)

      if not self.validateEditColor( str(self.editLine169.text()), 'line 16:9 color'): return
      if not self.validateEditColor( str(self.editLine235.text()), 'line 2.35 color'): return

      self.cmdField.clear()
      if self.inputPattern is None:
         self.cmdField.setText('Specify input sequence.')
         return

      self.StereoDuplicate = self.cStereoDuplicate.isChecked()

      if self.cAutoTitles.isChecked(): self.editShot.clear()
      if self.cAutoOutputName.isChecked():
         self.editOutputName.clear()

      project = '%s' % self.editProject.text()
      if Options.project == '':
          if self.cAutoTitles.isChecked() or project == '':
            if sys.platform.find('win') == 0:
               pat_split = self.inputPattern.upper().split('\\')
               if len(pat_split) > 4: project = pat_split[4]
               else: project = pat_split[-1]
            else:
               pat_split = self.inputPattern.upper().split('/')
               if len(pat_split) > 3: project = pat_split[3]
               else: project = pat_split[-1]
            self.editProject.setText( project)

      shot = '%s' % self.editShot.text()
      if self.cAutoTitles.isChecked() or shot == '':
         shot = os.path.basename( self.inputPattern)[ : os.path.basename( self.inputPattern).find('.')]
         self.editShot.setText( shot)

      version = '%s' % self.editVersion.text()
      if self.cAutoTitles.isChecked() or version == '':
         version = os.path.basename( os.path.dirname(self.inputPattern))
         self.editVersion.setText( version)

      company  = '%s' % self.editCompany.text()
      artist   = '%s' % self.editArtist.text()
      activity = '%s' % self.editActivity.text()
      comments = '%s' % self.editComments.text()
      suffix   = '%s' % self.editOutputSuffix.text()
      font     = '%s' % self.editFont.text()

      outdir = '%s' % self.editOutputDir.text()
      if outdir == '':
         outdir = os.path.dirname( os.path.dirname( self.inputPattern))
         self.editOutputDir.setText( outdir)

      outname = '%s' % self.editOutputName.text()
      if self.cAutoOutputName.isChecked() or outname == None or outname == '':
         outname = shot
         if activity != '': outname += '_' + activity
         if suffix   != '': outname += '_' + suffix
         if version  != '': outname += '_' + version
         outname += time.strftime('_%y%m%d')
         self.editOutputName.setText( outname)

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
      format = self.cbFormat.itemData( self.cbFormat.currentIndex()).toString()
      if not format.isEmpty():
         ts = self.cbTemplateS.currentText()
         tf = self.cbTemplateF.currentText()
         cmd += ' -r %s' % format
         cmd += ' -g %.2f' % self.dsbGamma.value()
         if ts != '': cmd += ' -s "%s"' % ts
         if tf != '': cmd += ' -t "%s"' % tf
         if project  != '': cmd += ' --project "%s"'  % project
         if shot     != '': cmd += ' --shot "%s"'     % shot
         if version  != '': cmd += ' --ver "%s"'      % version
         if company  != '': cmd += ' --company "%s"'  % company
         if artist   != '': cmd += ' --artist "%s"'   % artist
         if activity != '': cmd += ' --activity "%s"' % activity
         if comments != '': cmd += ' --comments "%s"' % comments
         if font     != '': cmd += ' --font "%s"'     % font
         if self.cTime.isChecked(): cmd += ' --addtime'
         cacher = self.cbCacher169.itemData( self.cbCacher169.currentIndex()).toString()
         if cacher != '0': cmd += ' --draw169 %s' % cacher
         cacher = self.cbCacher235.itemData( self.cbCacher235.currentIndex()).toString()
         if cacher != '0': cmd += ' --draw235 %s' % cacher
         if not self.editLine169.text().isEmpty(): cmd += ' --line169 "%s"' % self.editLine169.text()
         if not self.editLine235.text().isEmpty(): cmd += ' --line235 "%s"' % self.editLine235.text()
         if self.cLogo.isChecked():
            cmd += ' --logopath "%s"' % logopath
            cmd += ' --logosize %dx%d' % ( self.sbLogoSizeX.value(), self.sbLogoSizeY.value())
            cmd += ' --logograv %s' % self.cbLogoGravity.currentText()
      if self.cDateOutput.isChecked(): cmd += ' --datesuffix'
      if self.cTimeOutput.isChecked(): cmd += ' --timesuffix'
      if self.StereoDuplicate and self.inputPattern2 is None:
         cmd += ' --stereo'
      if self.cAfanasy.isChecked() and not self.cAfOneTask.isChecked():
         cmd += ' -A'
         if self.sbAfCapConvert.value() != -1: cmd += ' --afconvcap %d' % self.sbAfCapConvert.value()
         if self.sbAfCapEncode.value()  != -1: cmd += ' --afenccap %d' % self.sbAfCapEncode.value()
      if Options.debug: cmd += ' --debug'

      cmd += ' "%s"' % self.inputPattern
      if self.inputPattern2 is not None: cmd += ' "%s"' % self.inputPattern2
      cmd += ' "%s"' % os.path.join( outdir, outname)

      self.command = cmd
      self.cmdField.setText( cmd)
      self.evaluated = True
      self.btnStart.setEnabled( True)

   def execute( self):
      if not self.evaluated: return
      if len( self.command) == 0: return

      if self.cAfanasy.isChecked() and self.cAfOneTask.isChecked():
         self.btnStart.setEnabled( False)
         try:
            af = __import__('af', globals(), locals(), [])
         except:
            error = str(sys.exc_info()[1])
            print error
            self.cmdField.setText('Unable to import Afanasy Python module:\n' + error)
            return
         reload(af)
         job = af.Job(('%s' % self.editOutputName.text()).encode('utf-8') + ' mavishka')
         block = af.Block('mavishky')
         if self.sbAfPriority.value()  != -1: job.setPriority(    self.sbAfPriority.value())
         if self.sbAfMaxHosts.value()  != -1: job.setMaxHosts(    self.sbAfMaxHosts.value())
         if self.sbAfCapacity.value()  != -1: block.setCapacity(  self.sbAfCapacity.value())
         hostsmask         = '%s' % self.editAfHostsMask.text()
         hostsmaskexclude  = '%s' % self.editAfHostsMaskExclude.text()
         dependmask        = '%s' % self.editAfDependMask.text()
         dependmaskglobal  = '%s' % self.editAfDependMaskGlobal.text()
         if hostsmask        != '': job.setHostsMask(        hostsmask.encode('utf-8')        )
         if hostsmaskexclude != '': job.setHostsMaskExclude( hostsmaskexclude.encode('utf-8') )
         if dependmask       != '': job.setDependMask(       dependmask.encode('utf-8')       )
         if dependmaskglobal != '': job.setDependMaskGlobal( dependmaskglobal.encode('utf-8') )
         datetime = self.editAfTime.dateTime()
         if datetime > QtCore.QDateTime.currentDateTime(): job.setWaitTime( datetime.toTime_t())
         if self.cAfPause.isChecked(): job.pause()
         job.setNeedOS('')
         job.blocks.append( block)
         task = af.Task(('%s' % self.editOutputName.text()).encode('utf-8'))
         task.setCommand( self.command.encode('utf-8'))

         block.tasks.append( task)
         if job.send(): self.cmdField.setText('Afanasy job was successfully sent.')
         else:          self.cmdField.setText('Unable to send job to Afanasy server.')
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
      self.cmdField.setText('Command finished successfully.')

   def processoutput( self):
      output = self.process.readAll()
      print ('%s' % output),
      self.cmdField.insertPlainText( QtCore.QString( output))
      self.cmdField.moveCursor( QtGui.QTextCursor.End)

   def processStop( self):
      self.process.terminate()
      if sys.platform.find('win') == 0:
         self.process.kill()
      self.cmdField.setText('Stopped.')

app = QtGui.QApplication( sys.argv)
icon = QtGui.QIcon( os.path.join( os.path.join (DialogPath, 'icons'), 'icon.png'))
app.setWindowIcon( icon)
dialog = Dialog()
dialog.show()
app.exec_()
