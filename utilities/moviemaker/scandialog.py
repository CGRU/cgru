#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

import cgruconfig
import cgruutils

from Qt import QtCore, QtGui, QtWidgets

# Command arguments:

from optparse import OptionParser
Parser = OptionParser(usage="%prog [options] InputFolder OutputFolder\ntype \"%prog -h\" for help", version="%prog 1.0")
Parser.add_option('-c', '--codec',     dest='codec',        type  ='string',     default='h264_mid.ffmpeg',help='Default codec preset')
Parser.add_option('-f', '--format',    dest='format',       type  ='string',     default='1280x720',  help='Resolution')
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

Options, args = Parser.parse_args()

InputFolder = ''
OutputFolder = '..'
if len(args) > 0:
    InputFolder = args[0]
if len(args) > 1:
    OutputFolder = args[1]

Extensions = ['jpg', 'dpx', 'cin', 'exr', 'tga', 'tif', 'png']

DateTimeFormat = 'yy.MM.dd HH:mm'

# Initializations:
DialogPath = os.path.dirname(os.path.abspath(sys.argv[0]))
TemplatesPath = os.path.join(DialogPath, 'templates')
CodecsPath = os.path.join(DialogPath, 'codecs')
FormatsPath = os.path.join(DialogPath, 'formats')
Encoders = ['ffmpeg', 'mencoder']

# Process formats:
FormatNames = []
FormatValues = []
FormatFiles = []
allFiles = os.listdir(FormatsPath)
for afile in allFiles:
    afile = os.path.join(FormatsPath, afile)
    if os.path.isfile(afile):
        FormatFiles.append(afile)

FormatFiles.sort()
for afile in FormatFiles:
    with open(afile) as f:
        FormatNames.append(f.readline().strip())
        FormatValues.append(f.readline().strip())

if not Options.format in FormatValues:
    FormatValues.append(Options.format)
    FormatNames.append(Options.format)

# Process templates:
Templates = ['']
Template = 0
if os.path.isdir(TemplatesPath):
    files = os.listdir(TemplatesPath)
    files.sort()
    index = 0
    for afile in files:
        if afile[0] == '.':
            continue
        index += 1
        Templates.append(afile)
        if afile == Options.template:
            Template = index


# Process codecs:
CodecNames = []
CodecFiles = []
allFiles = os.listdir(CodecsPath)
for afile in allFiles:
    afile = os.path.join(CodecsPath, afile)
    if os.path.isfile(afile):
        parts = afile.split('.')
        if len(parts):
            if parts[len(parts) - 1] in Encoders:
                CodecFiles.append(afile)

CodecFiles.sort()
for afile in CodecFiles:
    with open(afile) as f:
        name = f.readline().strip()
    CodecNames.append(name)


class Dialog(QtWidgets.QWidget):
    def __init__(self):
        QtWidgets.QWidget.__init__(self)
        self.constructed = False

        self.evaluated = False
        self.test = False

        self.setWindowTitle(
            'Scan Scan - CGRU ' + cgruconfig.VARS['CGRU_VERSION'])
        self.mainLayout = QtWidgets.QVBoxLayout(self)

        self.tabwidget = QtWidgets.QTabWidget(self)
        self.generalwidget = QtWidgets.QWidget(self)
        self.tabwidget.addTab(self.generalwidget, 'General')
        self.generallayout = QtWidgets.QVBoxLayout(self.generalwidget)
        self.parameterswidget = QtWidgets.QWidget(self)
        self.tabwidget.addTab(self.parameterswidget, 'Parameters')
        self.parameterslayout = QtWidgets.QVBoxLayout(self.parameterswidget)
        self.mainLayout.addWidget(self.tabwidget)

        # General:
        self.lFormat = QtWidgets.QHBoxLayout()
        self.tFormat = QtWidgets.QLabel('Format:', self)
        self.tFormat.setToolTip('Movie resolution.\n'
                                'Format presets located in\n' + FormatsPath)
        self.cbFormat = QtWidgets.QComboBox(self)
        i = 0
        for format in FormatValues:
            self.cbFormat.addItem(FormatNames[i], format)
            if format == Options.format:
                self.cbFormat.setCurrentIndex(i)
            i += 1
        self.cbFormat.currentIndexChanged.connect( self.evaluate)
        self.tCodec = QtWidgets.QLabel('Codec:', self)
        self.tCodec.setToolTip('Codec presets located in\n' + CodecsPath)
        self.cbCodec = QtWidgets.QComboBox(self)
        i = 0
        for name in CodecNames:
            self.cbCodec.addItem(name, CodecFiles[i])
            if os.path.basename(CodecFiles[i]) == Options.codec:
                self.cbCodec.setCurrentIndex(i)
            i += 1
        self.cbCodec.currentIndexChanged.connect( self.evaluate)
        self.tFPS = QtWidgets.QLabel('FPS:', self)
        self.tFPS.setToolTip('Frame rate.')
        self.cbFPS = QtWidgets.QComboBox(self)
        self.cbFPS.addItem('24')
        self.cbFPS.addItem('25')
        self.cbFPS.addItem('30')
        self.cbFPS.setCurrentIndex(0)
        self.cbFPS.currentIndexChanged.connect( self.evaluate)
        self.lFormat.addWidget(self.tFormat)
        self.lFormat.addWidget(self.cbFormat)
        self.lFormat.addWidget(self.tFPS)
        self.lFormat.addWidget(self.cbFPS)
        self.lFormat.addWidget(self.tCodec)
        self.lFormat.addWidget(self.cbCodec)
        self.generallayout.addLayout(self.lFormat)

        self.lInput = QtWidgets.QHBoxLayout()
        self.tInput = QtWidgets.QLabel('Scan Folder', self)
        self.lInput.addWidget(self.tInput)
        self.editInput = QtWidgets.QLineEdit(InputFolder, self)
        self.editInput.textEdited.connect( self.evaluate)
        self.lInput.addWidget(self.editInput)
        self.btnInputBrowse = QtWidgets.QPushButton('Browse', self)
        self.btnInputBrowse.pressed.connect( self.inputBrowse)
        self.lInput.addWidget(self.btnInputBrowse)
        self.generallayout.addLayout(self.lInput)

        self.lExtensions = QtWidgets.QHBoxLayout()
        self.tExtensions = QtWidgets.QLabel('Search extensions:', self)
        tooltip = 'Comma separated list.\n' \
                  'Leave empty to find all known:\n'
        for ext in Extensions:
            tooltip += ext + ' '
        self.tExtensions.setToolTip(tooltip)
        self.lExtensions.addWidget(self.tExtensions)
        self.editExtensions = QtWidgets.QLineEdit(Options.extensions, self)
        self.editExtensions.editingFinished.connect( self.evaluate)
        self.lExtensions.addWidget(self.editExtensions)
        self.generallayout.addLayout(self.lExtensions)

        self.lInclude = QtWidgets.QHBoxLayout()
        self.tInclude = QtWidgets.QLabel('Include pattern:', self)
        self.lInclude.addWidget(self.tInclude)
        self.editInclude = QtWidgets.QLineEdit(Options.include, self)
        self.editInclude.textEdited.connect( self.evaluate)
        self.lInclude.addWidget(self.editInclude)
        self.generallayout.addLayout(self.lInclude)

        self.lExclude = QtWidgets.QHBoxLayout()
        self.tExclude = QtWidgets.QLabel('Exclude pattern:', self)
        self.lExclude.addWidget(self.tExclude)
        self.editExclude = QtWidgets.QLineEdit(Options.exclude, self)
        self.editExclude.textEdited.connect( self.evaluate)
        self.lExclude.addWidget(self.editExclude)
        self.generallayout.addLayout(self.lExclude)

        self.lDateTime = QtWidgets.QHBoxLayout()
        self.cDateTime = QtWidgets.QCheckBox('Skip folders ealier than:', self)
        self.cDateTime.stateChanged.connect( self.evaluate)
        self.lDateTime.addWidget(self.cDateTime)
        self.eDateTime = QtWidgets.QDateTimeEdit(
            QtCore.QDateTime.currentDateTime(), self)
        self.eDateTime.setCalendarPopup(True)
        self.eDateTime.setDisplayFormat(DateTimeFormat)
        self.eDateTime.dateTimeChanged.connect( self.evaluate)
        self.lDateTime.addWidget(self.eDateTime)
        self.generallayout.addLayout(self.lDateTime)

        self.lOutput = QtWidgets.QHBoxLayout()
        self.tOutput = QtWidgets.QLabel('Output Folder:', self)
        self.lOutput.addWidget(self.tOutput)
        self.editOutput = QtWidgets.QLineEdit(OutputFolder, self)
        self.editOutput.editingFinished.connect( self.evaluate)
        self.lOutput.addWidget(self.editOutput)
        self.btnOutputBrowse = QtWidgets.QPushButton('Browse', self)
        self.btnOutputBrowse.pressed.connect( self.browseOutput)
        self.lOutput.addWidget(self.btnOutputBrowse)
        self.generallayout.addLayout(self.lOutput)


        # Parameters:
        self.cAbsPath = QtWidgets.QCheckBox(
            'Prefix movies names with images absolute input files path',
            self
        )
        self.cAbsPath.setChecked(Options.abspath)
        self.cAbsPath.stateChanged.connect( self.evaluate)
        self.parameterslayout.addWidget(self.cAbsPath)

        self.lTemplates = QtWidgets.QHBoxLayout()
        self.tTemplate = QtWidgets.QLabel('Frame Template:', self)
        self.tTemplate.setToolTip('Frame template.\n'
                                  'Templates are located in\n' + TemplatesPath)
        self.cbTemplate = QtWidgets.QComboBox(self)
        for template in Templates:
            self.cbTemplate.addItem(template)
        self.cbTemplate.setCurrentIndex(Template)
        self.lTemplates.addWidget(self.tTemplate)
        self.lTemplates.addWidget(self.cbTemplate)
        self.cbTemplate.currentIndexChanged.connect( self.evaluate)
        self.parameterslayout.addLayout(self.lTemplates)

        self.lAspect = QtWidgets.QHBoxLayout()
        self.lAspect.addWidget(QtWidgets.QLabel('Input Images Aspect', self))
        self.dsbAspect = QtWidgets.QDoubleSpinBox(self)
        self.dsbAspect.setRange(-1.0, 10.0)
        self.dsbAspect.setDecimals(6)
        self.dsbAspect.setValue(Options.aspect_in)
        self.dsbAspect.valueChanged.connect( self.evaluate)
        self.lAspect.addWidget(self.dsbAspect)
        self.lAspect.addWidget(QtWidgets.QLabel(' (-1 = no changes) ', self))
        self.parameterslayout.addLayout(self.lAspect)

        self.lAutoAspect = QtWidgets.QHBoxLayout()
        self.tAutoAspect = QtWidgets.QLabel('Auto Input Aspect', self)
        self.tAutoAspect.setToolTip(
            'Images with width/height ratio > this value will be '
            'treated as 2:1.'
        )
        self.lAutoAspect.addWidget(self.tAutoAspect)
        self.dsbAutoAspect = QtWidgets.QDoubleSpinBox(self)
        self.dsbAutoAspect.setRange(-1.0, 10.0)
        self.dsbAutoAspect.setDecimals(3)
        self.dsbAutoAspect.setValue(Options.aspect_auto)
        self.dsbAutoAspect.valueChanged.connect( self.evaluate)
        self.lAutoAspect.addWidget(self.dsbAutoAspect)
        self.lAutoAspect.addWidget(QtWidgets.QLabel(' (-1 = no changes) ', self))
        self.parameterslayout.addLayout(self.lAutoAspect)

        self.gCorrectionSettings = QtWidgets.QGroupBox('Image Correction')
        self.lCorr = QtWidgets.QHBoxLayout()
        self.gCorrectionSettings.setLayout(self.lCorr)
        self.tGamma = QtWidgets.QLabel('Gamma:', self)
        self.dsbGamma = QtWidgets.QDoubleSpinBox(self)
        self.dsbGamma.setRange(0.1, 10.0)
        self.dsbGamma.setDecimals(1)
        self.dsbGamma.setSingleStep(0.1)
        self.dsbGamma.setValue(1.0)
        self.dsbGamma.valueChanged.connect( self.evaluate)
        self.lCorr.addWidget(self.tGamma)
        self.lCorr.addWidget(self.dsbGamma)
        self.parameterslayout.addWidget(self.gCorrectionSettings)


        # Bottom tab:
        self.cmdField = QtWidgets.QTextEdit(self)
        self.cmdField.setReadOnly(True)
        self.mainLayout.addWidget(self.cmdField)

        self.lProcess = QtWidgets.QHBoxLayout()
        self.btnStart = QtWidgets.QPushButton('Start', self)
        self.btnStart.setEnabled(False)
        self.btnStart.pressed.connect( self.execute)
        self.btnStop = QtWidgets.QPushButton('Stop', self)
        self.btnStop.setEnabled(False)
        self.btnStop.pressed.connect( self.processStop)
        self.btnTest = QtWidgets.QPushButton('Test', self)
        self.btnTest.setEnabled(False)
        self.btnTest.pressed.connect( self.executeTest)
        self.lProcess.addWidget(self.btnTest)
        self.lProcess.addWidget(self.btnStart)
        self.lProcess.addWidget(self.btnStop)
        self.mainLayout.addLayout(self.lProcess)

        self.lAfanasy = QtWidgets.QHBoxLayout()
        self.cAfanasy = QtWidgets.QCheckBox('Afanasy', self)
        self.cAfanasy.setChecked(Options.afanasy != 0)
        self.cAfanasy.stateChanged.connect( self.afanasy)
        self.tAfCapacity = QtWidgets.QLabel('Capacity:', self)
        self.sbAfCapacity = QtWidgets.QSpinBox(self)
        self.sbAfCapacity.setRange(-1, 1000000)
        self.sbAfCapacity.setValue(Options.afanasy)
        self.sbAfCapacity.valueChanged.connect( self.evaluate)
        self.tAfMaxHosts = QtWidgets.QLabel('Maximum Hosts:', self)
        self.sbAfMaxHosts = QtWidgets.QSpinBox(self)
        self.sbAfMaxHosts.setRange(-1, 1000000)
        self.sbAfMaxHosts.setValue(Options.maxhosts)
        self.sbAfMaxHosts.valueChanged.connect( self.evaluate)
        self.cAfPause = QtWidgets.QCheckBox('Pause', self)
        self.lAfanasy.addWidget(self.cAfanasy)
        self.lAfanasy.addWidget(self.tAfCapacity)
        self.lAfanasy.addWidget(self.sbAfCapacity)
        self.lAfanasy.addWidget(self.tAfMaxHosts)
        self.lAfanasy.addWidget(self.sbAfMaxHosts)
        self.lAfanasy.addWidget(self.cAfPause)
        self.mainLayout.addLayout(self.lAfanasy)

        self.constructed = True

        self.afanasy()
        self.evaluate()

    def afanasy(self):
        enableAf = self.cAfanasy.isChecked()
        self.sbAfCapacity.setEnabled(enableAf)
        self.sbAfMaxHosts.setEnabled(enableAf)
        if enableAf is True:
            if self.sbAfCapacity.value() == 0:
                self.sbAfCapacity.setValue(-1)
        self.evaluate()

    def inputBrowse(self):
        folder = QtWidgets.QFileDialog.getExistingDirectory(
            self,
            'Choose a folder',
            self.editInput.text()
        )
        if len(folder):
            self.editInput.setText(folder)
            self.evaluate()

    def browseOutput(self):
        folder = QtWidgets.QFileDialog.getExistingDirectory(
            self,
            'Choose a folder',
            self.editOutput.text()
        )

        if len(folder):
            self.editOutput.setText(folder)
            self.evaluate()

    def evaluate(self):
        if not self.constructed:
            return
        self.evaluated = False
        self.btnStart.setEnabled(False)
        self.btnStop.setEnabled(False)
        self.btnTest.setEnabled(False)

        input = str(self.editInput.text())
        output = str(self.editOutput.text())

        if input == '':
            self.cmdField.setText('Select a folder to scan.')
            return

        if not os.path.isdir(input):
            self.cmdField.setText('Input folder does not exist.')
            return
        if output == '':
            output = input
            self.editOutput.setText(output)

        extensions = str(self.editExtensions.text())
        include = str(self.editInclude.text())
        exclude = str(self.editExclude.text())
        format = getComboBoxString(self.cbFormat)
        template = self.cbTemplate.currentText()

        cmd = 'scanscan.py'
        cmd = os.path.join(DialogPath, cmd)
        cmd = '"%s" "%s"' % (os.getenv('CGRU_PYTHONEXE', 'python'), cmd)
        cmd += ' -c %s' % getComboBoxString(self.cbCodec)
        cmd += ' -f %s' % self.cbFPS.currentText()
        cmd += ' -r %s' % format
        if self.dsbGamma.value() != 1.0:
            cmd += ' -g %.2f' % self.dsbGamma.value()

        if template != '':
            cmd += ' -t "%s"' % template

        if extensions != '':
            cmd += ' -e "%s"' % extensions

        if include != '':
            cmd += ' --include "%s"' % include

        if exclude != '':
            cmd += ' --exclude "%s"' % exclude

        if self.cDateTime.isChecked():
            cmd += ' --after %d' % self.eDateTime.dateTime().toTime_t()

        if self.cAbsPath.isChecked():
            cmd += ' -a'

        if self.cAfanasy.isChecked():
            cmd += ' -A %d' % self.sbAfCapacity.value()
            cmd += ' -m %d' % self.sbAfMaxHosts.value()
            if self.cAfPause.isChecked():
                cmd += ' --pause'

        if self.dsbAspect.value() > 0:
            cmd += ' --aspect_in %f' % self.dsbAspect.value()

        if self.dsbAutoAspect.value() > 0:
            cmd += ' --aspect_auto %f' % self.dsbAutoAspect.value()

        if self.test:
            cmd += ' --test'

        cmd += ' "%s"' % self.editInput.text()
        cmd += ' "%s"' % self.editOutput.text()

        self.cmdField.setText(cmd)
        self.evaluated = True
        self.btnStart.setEnabled(True)
        self.btnTest.setEnabled(True)

    def executeTest(self):
        if not self.evaluated:
            return
        self.test = True
        self.execute()

    def execute(self):
        if not self.evaluated:
            return
        self.evaluate()
        self.command = str(self.cmdField.toPlainText())
        if len(self.command) == 0:
            return
        self.btnStart.setEnabled(False)
        self.btnStop.setEnabled(True)
        self.btnTest.setEnabled(False)
        self.cmdField.clear()
        self.process = QtCore.QProcess(self)
        self.process.setProcessChannelMode(QtCore.QProcess.MergedChannels)
        self.process.finished.connect( self.processfinished)
        self.process.readyRead.connect( self.processoutput)
        self.process.start(self.command)

    def processfinished(self, exitCode):
        print('Exit code = %d' % exitCode)
        self.btnStop.setEnabled(False)
        if exitCode != 0:
            return
        if not self.test:
            self.cmdField.setText(self.command)
        else:
            self.test = False
        self.btnTest.setEnabled(True)
        self.btnStart.setEnabled(True)

    def processoutput(self):
        output = self.process.readAll().data()
        if not isinstance(output, str):
            output = str(output, 'utf-8')
        output = output.strip()
        print(output)
        self.cmdField.insertPlainText(output + '\n')
        self.cmdField.moveCursor( QtGui.QTextCursor.End)

    def processStop(self):
        self.process.terminate()


def getComboBoxString(comboBox):
    data = comboBox.itemData(comboBox.currentIndex())
    if data is None:
        return ''
    if isinstance(data, str):
        return data
    if isinstance(data, unicode):
        return data
    return comboBox.itemData(comboBox.currentIndex()).toString()


app = QtWidgets.QApplication(sys.argv)
app.setWindowIcon( QtGui.QIcon(cgruutils.getIconFileName(Options.wndicon)))
dialog = Dialog()
dialog.show()
app.exec_()
