# -*- coding: utf-8 -*-
import os
import sys
import time

from Qt import QtCore, QtGui, QtWidgets

import cgruconfig
import cgruutils

import afcommon

from optparse import OptionParser

Parser = OptionParser(usage="usage: %prog [scene]", version="%prog 1.0")
Parser.add_option('-V', '--verbose', action='store_true', dest='verbose', default=False, help='Verbose mode')
Options, Args = Parser.parse_args()

# Initializations:
ScenesSequences = ['.ifd', '.vrscene', '.ass']
Scene = ''
if len(Args) > 0:
    Scene = Args[0]

FilePrefix = 'afstarter.'
FileSuffix = '.txt'
FileLast = 'last'
FileRecent = 'recent'


def getComboBoxString(comboBox):
    data = comboBox.itemData(comboBox.currentIndex())
    if data is None:
        return ''
    if isinstance(data, str):
        return data
    if isinstance(data, unicode):
        return data
    return str(comboBox.itemData(comboBox.currentIndex()).toString())
    # return comboBox.itemData( comboBox.currentIndex()).toString()


def getComboBoxIndexFromData(comboBox, data):
    dataDict = dict()
    value = int()
    for i in range(comboBox.count()):
        # dataDict[str(comboBox.itemData( i).toString())] = int( i)
        dataDict[comboBox.itemData(i)] = int(i)
    if data in dataDict:
        value = int(dataDict[str(data)])
    if value is None:
        return 0
    if isinstance(value, int):
        return value
    return value


def fileTypeList():
    fileType_list = list()
    # Syntax:
    # ( [ 'file_ext', 'file_ext' ], 'internal_name', 'UI Name' )
    fileType_list.append((['shk'],      'shake',        'Shake'))
    fileType_list.append((['blend'],    'blender',      'Blender'))
    fileType_list.append((['nk'],       'nuke',         'Nuke'))
    fileType_list.append((['ntp'],      'natron',       'Natron'))
    fileType_list.append((['hip'],      'houdini',      'Houdini'))
    fileType_list.append((['ifd'],      'mantra',       'Mantra'))
    fileType_list.append((['mb', 'ma'], 'maya',         'Maya'))
    fileType_list.append((['mb', 'ma'], 'maya_mental',  'Mental Ray'))
    fileType_list.append((['mb', 'ma'], 'maya_arnold',  'MtoA'))
    fileType_list.append((['mb', 'ma'], 'maya_redshift', 'Redshift'))
    fileType_list.append((['mb', 'ma'], 'maya_delight', '3Delight'))
    fileType_list.append((['scn'],      'xsi',          'Softimage'))
    fileType_list.append((['max'],      'max',          '3dsmax'))
    fileType_list.append((['aep'],      'afterfx',      'After Effects'))
    fileType_list.append((['render'],   'clarisse',     'Isotropix Clarisse'))
    return fileType_list


def fileType_internalNameToIndex(internalName):
    index = int()
    filetypes = fileTypeList()
    for i in range(len(filetypes)):
        filetype = filetypes[i]
        filetypeName = filetype[1]
        filetypeUIName = filetype[2]
        if filetypeName == internalName:
            index = i
    return index


def fileType_internalNameToUIName(internalName):
    uiName = str()
    filetypes = fileTypeList()
    for i in filetypes:
        filetypeName = i[1]
        filetypeUIName = i[2]
        if filetypeName == internalName:
            uiName = filetypeUIName
    return uiName


def fileType_uiNameToInternalName(uiName):
    internalName = str()
    filetypes = fileTypeList()
    for i in filetypes:
        filetypeName = i[1]
        filetypeUIName = i[2]
        if filetypeUIName == uiName:
            internalName = filetypeName
    return internalName


def labelNodeNameDict():
    labelNode_name = dict()
    labelNode_name['default']      = 'Node/Camera:'
    labelNode_name['houdini']      = 'ROP:'
    labelNode_name['natron']       = 'Write Node:'
    labelNode_name['nuke']         = 'Write Node:'
    labelNode_name['maya']         = 'Camera:'
    labelNode_name['maya_delight'] = 'Camera:'
    labelNode_name['maya_mental']  = 'Camera:'
    labelNode_name['maya_arnold']  = 'Camera:'
    labelNode_name['maya_redshift']  = 'Camera:'
    labelNode_name['3dsmax']       = 'Camera:'
    labelNode_name['afterfx']      = 'Composition:'
    labelNode_name['clarisse']     = 'Image:'
    return labelNode_name


def labelNodeTooltipsDict():
    labelNode_tooltip = dict()
    labelNode_tooltip['default'] = ('Houdini ROP\n'
                                    'Natron write node\n'
                                    'Nuke write node\n'
                                    'Maya camera\n'
                                    '3DSMAX camera\n'
                                    'AfterFX composition')
    labelNode_tooltip['houdini']      = 'Houdini ROP'
    labelNode_tooltip['natron']       = 'Natron write node\nExample: \'Write1\''
    labelNode_tooltip['nuke']         = 'Nuke write node\nExample: \'Write1\''
    labelNode_tooltip['maya']         = 'Maya camera\nExample: \'persp\''
    labelNode_tooltip['maya_delight'] = 'Maya camera\nExample: \'persp\''
    labelNode_tooltip['maya_mental']  = 'Maya camera\nExample: \'persp\''
    labelNode_tooltip['maya_arnold']  = 'Maya camera\nExample: \'persp\''
    labelNode_tooltip['maya_redshift']  = 'Maya camera\nExample: \'persp\''
    labelNode_tooltip['3dsmax']       = '3DSMAX camera'
    labelNode_tooltip['afterfx']      = 'AfterFX composition\nExample: \'Comp 1\''
    labelNode_tooltip['clarisse']     = 'Image item path\nExample:\nscene/image'
    return labelNode_tooltip


def labelTakeNameDict():
    labelTake_name = dict()
    labelTake_name['default']      = 'Take/Layer/Pass/Batch:'
    labelTake_name['houdini']      = 'Take:'
    labelTake_name['xsi']          = 'Pass:'
    labelTake_name['natron']       = 'Not Used:'
    labelTake_name['nuke']         = 'Not Used:'
    labelTake_name['maya']         = 'Layer:'
    labelTake_name['maya_delight'] = 'Render Pass:'
    labelTake_name['maya_mental']  = 'Layer:'
    labelTake_name['maya_arnold']  = 'Layer:'
    labelTake_name['maya_redshift']  = 'Layer:'
    labelTake_name['3dsmax']       = 'Batch:'
    labelTake_name['afterfx']      = 'render settings template:'
    labelTake_name['clarisse']     = 'Format:'
    return labelTake_name


def labelTakeTooltipsDict():
    labelTake_tooltip = dict()
    labelTake_tooltip['default'] = ('Houdini take\n'
                                    'SoftImage pass\n'
                                    'Maya layer\n'
                                    '3DFM render pass\n'
                                    '3DSMAX batch\n'
                                    'AfterFX render settings template')
    labelTake_tooltip['houdini']      = 'Houdini take'
    labelTake_tooltip['xsi']          = 'SoftImage pass'
    labelTake_tooltip['natron']       = '- Not used -'
    labelTake_tooltip['nuke']         = '- Not used -'
    labelTake_tooltip['maya']         = 'Maya render layer'
    labelTake_tooltip['maya_delight'] = '3Delight for Maya Render Pass'
    labelTake_tooltip['maya_mental']  = 'Maya render layer'
    labelTake_tooltip['maya_arnold']  = 'Maya render layer'
    labelTake_tooltip['maya_redshift']  = 'Maya render layer'
    labelTake_tooltip['3dsmax']       = '3DSMAX batch'
    labelTake_tooltip['afterfx']      = 'AfterFX render settings template\nExample: \'Best Settings\''
    labelTake_tooltip['clarisse']     = 'Ouput images format name\nExample:\njpg, exr16, exr32, ...\nSee available format names in docs.'
    return labelTake_tooltip


# Dialog class
class Dialog(QtWidgets.QWidget):
    def __init__(self):

        # Main Window
        QtWidgets.QWidget.__init__(self)
        self.constructed = False
        self.evaluated = False
        self.output = ''
        self.setWindowTitle('Afanasy Starter   CGRU %s' %
                            os.getenv('CGRU_VERSION', ''))
        self.fields = dict()
        self.labels = dict()

        # Get File Type dependant dicts
        self.labelNode_name = labelNodeNameDict()
        self.labelNode_tooltip = labelNodeTooltipsDict()
        self.labelTake_name = labelTakeNameDict()
        self.labelTake_tooltip = labelTakeTooltipsDict()

        # Top level layouts:
        topLayout = QtWidgets.QVBoxLayout(self)
        tabwidget = QtWidgets.QTabWidget(self)
        topLayout.addWidget(tabwidget)

        # General Tab:
        generalwidget = QtWidgets.QWidget(self)
        tabwidget.addTab(generalwidget, 'Scene')
        generallayout = QtWidgets.QVBoxLayout(generalwidget)

        # Job Tab:
        jobwidget = QtWidgets.QWidget(self)
        tabwidget.addTab(jobwidget, 'Job')
        joblayout = QtWidgets.QVBoxLayout(jobwidget)

        # Advanced Tab:
        auxwidget = QtWidgets.QWidget(self)
        tabwidget.addTab(auxwidget, 'Advanced')
        advlayout = QtWidgets.QVBoxLayout(auxwidget)

        # Service Type:
        lFileType = QtWidgets.QHBoxLayout()
        generallayout.addLayout(lFileType)
        lFileType.addWidget(QtWidgets.QLabel('Override Service Type:', self))
        self.fields['servicetype'] = QtWidgets.QComboBox(self)
        lFileType.addWidget(self.fields['servicetype'])
        self.fields['servicetype'].currentIndexChanged.connect( self.setFileType)

        # Scene:
        lScene = QtWidgets.QHBoxLayout()
        generallayout.addLayout(lScene)
        lScene.addWidget(QtWidgets.QLabel('File:', self))
        self.fields['scenefile'] = QtWidgets.QLineEdit(Scene, self)
        lScene.addWidget(self.fields['scenefile'])
        self.fields['scenefile'].editingFinished.connect( self.evaluate)
        bBrowseScene = QtWidgets.QPushButton('Browse', self)
        lScene.addWidget(bBrowseScene)
        bBrowseScene.pressed.connect( self.browseScene)

        # Working Directory:
        lWDir = QtWidgets.QHBoxLayout()
        generallayout.addLayout(lWDir)
        lWDir.addWidget(QtWidgets.QLabel('Working Directory/Project:', self))
        self.fields['wdir'] = QtWidgets.QLineEdit(self)
        lWDir.addWidget(self.fields['wdir'])
        self.fields['wdir'].editingFinished.connect( self.evaluate)
        self.fields['scenewdir'] = QtWidgets.QCheckBox('Use Scene Folder', self)
        self.fields['scenewdir'].setChecked(True)
        self.fields['scenewdir'].stateChanged.connect( self.evaluate)
        lWDir.addWidget(self.fields['scenewdir'])

        # Output images:
        lImages = QtWidgets.QHBoxLayout()
        generallayout.addLayout(lImages)
        lImages.addWidget(QtWidgets.QLabel('Output Images:', self))
        self.fields['outimages'] = QtWidgets.QLineEdit(self)
        lImages.addWidget(self.fields['outimages'])
        self.fields['outimages'].editingFinished.connect( self.evaluate)
        lImages.addWidget(QtWidgets.QLabel('Browse'))
        bBrowseOutImages = QtWidgets.QPushButton('File', self)
        lImages.addWidget(bBrowseOutImages)
        bBrowseOutImages.pressed.connect( self.browseOutImages)
        bBrowseOutFolder = QtWidgets.QPushButton('Dir', self)
        lImages.addWidget(bBrowseOutFolder)
        bBrowseOutFolder.pressed.connect( self.browseOutFolder)

        # Frames:
        lFrames = QtWidgets.QHBoxLayout()
        generallayout.addLayout(lFrames)
        lFrames.addWidget(QtWidgets.QLabel('Frames:', self))
        self.fields['framestart'] = QtWidgets.QSpinBox(self)
        self.fields['framestart'].setRange(-1000000000, 1000000000)
        self.fields['framestart'].setValue(1)
        self.fields['framestart'].valueChanged.connect( self.evaluate)
        lFrames.addWidget(self.fields['framestart'])

        self.fields['frameend'] = QtWidgets.QSpinBox(self)
        self.fields['frameend'].setRange(-1000000000, 1000000000)
        self.fields['frameend'].setValue(1)
        self.fields['frameend'].valueChanged.connect( self.evaluate)
        lFrames.addWidget(self.fields['frameend'])

        lFrames.addWidget(QtWidgets.QLabel('By:', self))
        self.fields['frameby'] = QtWidgets.QSpinBox(self)
        lFrames.addWidget(self.fields['frameby'])
        self.fields['frameby'].valueChanged.connect( self.evaluate)
        self.fields['frameby'].setRange(1, 1000000000)

        lFrames.addWidget(QtWidgets.QLabel('FPT:', self))
        self.fields['framespt'] = QtWidgets.QSpinBox(self)
        lFrames.addWidget(self.fields['framespt'])
        self.fields['framespt'].valueChanged.connect( self.evaluate)
        self.fields['framespt'].setRange(1, 1000000000)
        self.fields['framespt'].setToolTip('Frames per task.')

        lFrames.addWidget(QtWidgets.QLabel('Seq:', self))
        self.fields['frameseq'] = QtWidgets.QSpinBox(self)
        lFrames.addWidget(self.fields['frameseq'])
        self.fields['frameseq'].valueChanged.connect( self.evaluate)
        self.fields['frameseq'].setRange(-1000000, 1000000)
        self.fields['frameseq'].setValue(1)
        self.fields['frameseq'].setToolTip('Solve task with this step at first.')


        # Node / Camera / Take:
        lNode = QtWidgets.QHBoxLayout()
        generallayout.addLayout(lNode)
        self.labels['node'] = QtWidgets.QLabel(self.labelNode_name['default'])
        lNode.addWidget(self.labels['node'])
        self.labels['node'].setToolTip(self.labelNode_tooltip['default'])
        self.fields['node'] = QtWidgets.QLineEdit(self)
        lNode.addWidget(self.fields['node'])
        self.fields['node'].textEdited.connect( self.evaluate)
        self.labels['take'] = QtWidgets.QLabel(self.labelTake_name['default'])
        lNode.addWidget(self.labels['take'])
        self.labels['take'].setToolTip(self.labelTake_tooltip['default'])
        self.fields['take'] = QtWidgets.QLineEdit(self)
        lNode.addWidget(self.fields['take'])
        self.fields['take'].textEdited.connect( self.evaluate)

        # Advanced:
        # OS Type:
        osLayout = QtWidgets.QHBoxLayout()
        advlayout.addLayout(osLayout)
        osLayout.addWidget(QtWidgets.QLabel('Operating System Type:'))
        self.fields['os'] = QtWidgets.QLineEdit(self)
        osLayout.addWidget(self.fields['os'])
        self.fields['os'].textEdited.connect( self.evaluate)

        # Tasks Command:
        exelayout = QtWidgets.QHBoxLayout()
        advlayout.addLayout(exelayout)
        label = QtWidgets.QLabel('Tasks Executable:')
        label.setToolTip('Override launching programm')
        exelayout.addWidget(label)
        self.fields['exec'] = QtWidgets.QLineEdit(self)
        exelayout.addWidget(self.fields['exec'])
        self.fields['exec'].textEdited.connect( self.evaluate)
        self.execBrowseButton = QtWidgets.QPushButton('Browse', self)
        exelayout.addWidget(self.execBrowseButton)
        self.execBrowseButton.pressed.connect( self.browseExec)

        # Extra Arguments:
        exarglayout = QtWidgets.QHBoxLayout()
        advlayout.addLayout(exarglayout)
        exarglayout.addWidget(QtWidgets.QLabel('Extra Arguments:'))
        self.fields['extrargs'] = QtWidgets.QLineEdit(self)
        exarglayout.addWidget(self.fields['extrargs'])
        self.fields['extrargs'].textEdited.connect( self.evaluate)

        # Preview:
        prvlayout = QtWidgets.QHBoxLayout()
        advlayout.addLayout(prvlayout)
        prvlayout.addWidget(QtWidgets.QLabel('Preview:'))
        self.fields['preview'] = QtWidgets.QLineEdit(self)
        prvlayout.addWidget(self.fields['preview'])
        self.fields['preview'].textEdited.connect( self.evaluate)

        # Job:
        lJobName = QtWidgets.QHBoxLayout()
        joblayout.addLayout(lJobName)
        lJobName.addWidget(QtWidgets.QLabel('Name:', self))
        self.fields['jobname'] = QtWidgets.QLineEdit(self)
        lJobName.addWidget(self.fields['jobname'])
        self.fields['jobname'].textEdited.connect( self.evaluate)
        self.fields['jobnamescene'] = QtWidgets.QCheckBox('Use Scene Name', self)
        lJobName.addWidget(self.fields['jobnamescene'])
        self.fields['jobnamescene'].setChecked(True)
        self.fields['jobnamescene'].stateChanged.connect( self.evaluate)

        # Capacity, max run tasks, priority, max tasks per host:
        lCapMax = QtWidgets.QHBoxLayout()
        joblayout.addLayout(lCapMax)
        lCapMax.addWidget(QtWidgets.QLabel('Capacity:', self))
        self.fields['capacity'] = QtWidgets.QSpinBox(self)
        lCapMax.addWidget(self.fields['capacity'])
        self.fields['capacity'].setRange(-1, 1000000)
        self.fields['capacity'].setValue(-1)
        self.fields['capacity'].valueChanged.connect( self.evaluate)
        lCapMax.addWidget(QtWidgets.QLabel('Maximum Running Tasks:', self))
        self.fields['maxruntasks'] = QtWidgets.QSpinBox(self)
        lCapMax.addWidget(self.fields['maxruntasks'])
        self.fields['maxruntasks'].setRange(-1, 1000000)
        self.fields['maxruntasks'].setValue(-1)
        self.fields['maxruntasks'].valueChanged.connect( self.evaluate)
        lCapMax.addWidget(QtWidgets.QLabel('Maximum Tasks Per Host:', self))
        self.fields['maxtasksperhost'] = QtWidgets.QSpinBox(self)
        lCapMax.addWidget(self.fields['maxtasksperhost'])
        self.fields['maxtasksperhost'].setRange(-1, 1000000)
        self.fields['maxtasksperhost'].setValue(-1)
        self.fields['maxtasksperhost'].valueChanged.connect( self.evaluate)
        lCapMax.addWidget(QtWidgets.QLabel('Priority:', self))
        self.fields['priority'] = QtWidgets.QSpinBox(self)
        lCapMax.addWidget(self.fields['priority'])
        self.fields['priority'].setRange(-1, 250)
        self.fields['priority'].setValue(-1)
        self.fields['priority'].valueChanged.connect( self.evaluate)

        # Depend Masks:
        lDepends = QtWidgets.QHBoxLayout()
        joblayout.addLayout(lDepends)
        lDepends.addWidget(QtWidgets.QLabel('Depend Mask:', self))
        self.fields['dependmask'] = QtWidgets.QLineEdit(self)
        lDepends.addWidget(self.fields['dependmask'])
        self.fields['dependmask'].textEdited.connect( self.evaluate)
        lDepends.addWidget(QtWidgets.QLabel('Global:', self))
        self.fields['dependglobal'] = QtWidgets.QLineEdit(self)
        lDepends.addWidget(self.fields['dependglobal'])
        self.fields['dependglobal'].textEdited.connect( self.evaluate)

        # Host Masks:
        lHostMasks = QtWidgets.QHBoxLayout()
        joblayout.addLayout(lHostMasks)
        lHostMasks.addWidget(QtWidgets.QLabel('Hosts Mask:', self))
        self.fields['hostsmask'] = QtWidgets.QLineEdit(self)
        lHostMasks.addWidget(self.fields['hostsmask'])
        self.fields['hostsmask'].textEdited.connect( self.evaluate)
        lHostMasks.addWidget(QtWidgets.QLabel('Exclude:', self))
        self.fields['hostsexclude'] = QtWidgets.QLineEdit(self)
        lHostMasks.addWidget(self.fields['hostsexclude'])
        self.fields['hostsexclude'].textEdited.connect( self.evaluate)

        # Presets:
        presetsLayout = QtWidgets.QHBoxLayout()
        topLayout.addLayout(presetsLayout)
        presetsLayout.addWidget(QtWidgets.QLabel('Recent:', self))
        self.cbRecent = QtWidgets.QComboBox(self)
        self.cbRecent.activated.connect( self.loadRecent)
        presetsLayout.addWidget(self.cbRecent)
        self.bBrowseLoad = QtWidgets.QPushButton('Load', self)
        presetsLayout.addWidget(self.bBrowseLoad)
        self.bBrowseLoad.pressed.connect( self.browseLoad)
        self.bBrowseSave = QtWidgets.QPushButton('Save', self)
        presetsLayout.addWidget(self.bBrowseSave)
        self.bBrowseSave.pressed.connect( self.browseSave)


        # Command Field:
        self.teCmd = QtWidgets.QTextEdit(self)
        topLayout.addWidget(self.teCmd)

        # Buttons:
        buttonsLayout = QtWidgets.QHBoxLayout()
        topLayout.addLayout(buttonsLayout)
        self.bStart = QtWidgets.QPushButton('&Start', self)
        buttonsLayout.addWidget(self.bStart)
        self.bStart.pressed.connect( self.start)
        self.fields['paused'] = QtWidgets.QCheckBox('Paused', self)
        buttonsLayout.addWidget(self.fields['paused'])
        self.fields['paused'].stateChanged.connect( self.evaluate)
        self.bRefresh = QtWidgets.QPushButton('&Refresh', self)
        buttonsLayout.addWidget(self.bRefresh)
        self.bRefresh.pressed.connect( self.evaluate)
        self.bQuitSave = QtWidgets.QPushButton('&Quit&&Store', self)
        buttonsLayout.addWidget(self.bQuitSave)
        self.bQuitSave.pressed.connect( self.quitsave)

        self.constructed = True

        # Set window icon:
        iconpath = cgruutils.getIconFileName('afanasy')
        if iconpath is not None:
            self.setWindowIcon( QtGui.QIcon(iconpath))

        # Refresh recent:
        self.refreshRecent()

        # Load last settings:
        if not self.load(FileLast):
            self.evaluate()

    def browseScene(self):
        path, fltr = QtWidgets.QFileDialog.getOpenFileName(
            self, 'Choose a file',
            self.fields['scenefile'].text()
        )
        if path == '': return
        self.fields['scenefile'].setText(os.path.normpath(path))
        self.fileChange()
        self.evaluate()

    def browseOutImages(self):
        path, fltr = QtWidgets.QFileDialog.getSaveFileName(
            self,
            'Choose a file',
            self.fields['outimages'].text()
        )
        if path == '': return
        self.fields['outimages'].setText(os.path.normpath(path))
        self.evaluate()

    def browseOutFolder(self):
        path = QtWidgets.QFileDialog.getExistingDirectory(
            self,
            'Choose a directory',
            self.fields['outimages'].text()
        )
        if path == '': return
        self.fields['outimages'].setText(os.path.normpath(path))
        self.evaluate()

    def browseExec(self):
        path, fltr = QtWidgets.QFileDialog.getOpenFileName(
            self,
            'Choose a file',
            self.fields['exec'].text()
        )
        if path == '': return
        self.fields['exec'].setText(os.path.normpath(path))
        self.evaluate()

    def fileChange(self):
        scene_filepath = str()
        try:
            scene_filepath = self.fields['scenefile'].text()
        except KeyError:
            return False

        if os.path.isfile(scene_filepath):
            realValue = getComboBoxString(self.fields['servicetype'])

            enabledFileTypes = list()
            theFileTypes = list()
            theFileTypesData = list()

            filetypes = fileTypeList()
            for i in range(len(filetypes)):
                filetype = filetypes[i]

                if type(filetype) != type(str()):
                    fileExt_list = filetype[0]
                    filetypeName = filetype[1]

                    for j in fileExt_list:
                        file_ext = '.' + j
                        if str(scene_filepath).endswith(file_ext):
                            enabledFileTypes.append(filetypeName)

            theFileTypes.append('No Override')
            theFileTypesData.append('none')
            if len(enabledFileTypes) >= 1:
                for enabledFileType in enabledFileTypes:
                    theFileTypes.append(
                        fileType_internalNameToUIName(enabledFileType))
                    theFileTypesData.append(enabledFileType)

            self.fields['servicetype'].clear()
            for i in range(len(theFileTypes)):
                self.fields['servicetype'].addItem(theFileTypes[i],
                                                   theFileTypesData[i])

            theFileTypeData = ''
            if (len(theFileTypesData) == 2) \
                    and not ((len(theFileTypesData) == 0)
                             or (len(theFileTypesData) == 1)):
                theFileTypeData = theFileTypesData[1]
            else:
                theFileTypeData = realValue

            # run file type dependant function
            self.fileTypeDependantText(theFileTypeData)
        else:
            return False
        return True

    def setFileType(self):
        real_value = getComboBoxString(self.fields['servicetype'])
        self.fileTypeDependantText(real_value)
        if self.evaluated:
            self.evaluate()
        return True

    def fileTypeDependantText(self, theFileTypeData):
        labelKeyName = 'node'
        try:
            self.labels[labelKeyName].setText(
                self.labelNode_name[theFileTypeData])
        except KeyError:
            self.labels[labelKeyName].setText(self.labelNode_name['default'])
        try:
            self.labels[labelKeyName].setToolTip(
                self.labelNode_tooltip[theFileTypeData]
            )
        except KeyError:
            self.labels[labelKeyName].setToolTip(
                self.labelNode_tooltip['default'])

        labelKeyName = 'take'
        try:
            self.labels[labelKeyName].setText(
                self.labelTake_name[theFileTypeData]
            )
        except KeyError:
            self.labels[labelKeyName].setText(self.labelTake_name['default'])
        try:
            self.labels[labelKeyName].setToolTip(
                self.labelTake_tooltip[theFileTypeData])
        except KeyError:
            self.labels[labelKeyName].setToolTip(
                self.labelTake_tooltip['default'])

        return True

    def quitsave(self):
        self.save(FileLast)
        self.close()

    def save(self, filename, fullPath=False):
        if not fullPath:
            filename = '%s%s%s' % (
                os.path.join(cgruconfig.VARS['HOME_CGRU'], FilePrefix),
                filename,
                FileSuffix
            )
        with open(filename, 'w') as f:
            for key in self.fields:
                value = ''
                if isinstance(self.fields[key], QtWidgets.QLineEdit):
                    value = str(self.fields[key].text())
                elif isinstance(self.fields[key], QtWidgets.QSpinBox):
                    value = str(self.fields[key].value())
                elif isinstance(self.fields[key], QtWidgets.QCheckBox):
                    value = str(int(self.fields[key].isChecked()))
                elif isinstance(self.fields[key], QtWidgets.QComboBox):
                    # value = str( self.fields[key].itemData( self.fields[key].currentIndex()).toString())
                    value = str(self.fields[key].itemData(
                        self.fields[key].currentIndex()))
                line = key + '=' + value
                f.write(line + '\n')

    def getRecentFilesList(self):
        allfiles = os.listdir(cgruconfig.VARS['HOME_CGRU'])
        recfiles = []
        for afile in allfiles:
            if afile.find(FilePrefix + FileRecent) >= 0:
                recfiles.append(afile)
        recfiles.sort()
        return recfiles

    def saveRecent(self):
        recfiles = self.getRecentFilesList()
        if len(recfiles) > 0:
            for afile in recfiles:
                if afile.find(self.fields['jobname'].text()) > len(
                                FilePrefix + FileRecent):
                    # print('os.remove("%s")' % os.path.join( cgruconfig.VARS['HOME_CGRU'], afile))
                    os.remove(
                        os.path.join(
                            cgruconfig.VARS['HOME_CGRU'],
                            afile
                        )
                    )
                    recfiles.remove(afile)
            numfiles = len(recfiles)
            if numfiles > 9:
                # print('os.remove("%s")' % os.path.join( cgruconfig.VARS['HOME_CGRU'], recfiles[-1]))
                os.remove(
                    os.path.join(
                        cgruconfig.VARS['HOME_CGRU'],
                        recfiles[-1]
                    )
                )
                del recfiles[-1]
            recfiles.reverse()
            index = len(recfiles)
            for afile in recfiles:
                pos = afile.find(FilePrefix + FileRecent)
                if pos < 0:
                    continue
                pos = len(FilePrefix + FileRecent)
                num = int(afile[pos])
                if num != index:
                    nextfile = afile[:pos] + str(index) + afile[pos + 1:]
                    afile = os.path.join(cgruconfig.VARS['HOME_CGRU'], afile)
                    nextfile = os.path.join(
                        cgruconfig.VARS['HOME_CGRU'],
                        nextfile
                    )
                    # print('os.rename("%s"->"%s")' % ( afile, nextfile))
                    os.rename(afile, nextfile)
                index -= 1
        afile = FileRecent + '0.' + self.fields['jobname'].text()
        self.save(afile)
        self.refreshRecent()

    def refreshRecent(self):
        self.cbRecent.activated.disconnect( self.loadRecent)
        self.cbRecent.clear()
        for afile in self.getRecentFilesList():
            if afile[: len(FilePrefix)] == FilePrefix:
                afile = afile[len(FilePrefix):]
            if afile[-len(FileSuffix):] == FileSuffix:
                afile = afile[: -len(FileSuffix)]
            short = afile
            if short[: len(FileRecent)] == FileRecent:
                short = short[len(FileRecent):]
            short = short[2:]
            if len(short) > 20:
                short = short[:10] + ' .. ' + short[-10:]
            self.cbRecent.addItem(short, afile)
        
        self.cbRecent.activated.connect( self.loadRecent)

    def loadRecent(self):
        self.load(getComboBoxString(self.cbRecent))

    def load(self, filename, fullPath=False):
        if not fullPath:
            filename = '%s%s%s' % (
                os.path.join(
                    cgruconfig.VARS['HOME_CGRU'],
                    FilePrefix
                ),
                filename,
                FileSuffix
            )

        if not os.path.isfile(filename):
            return False

        print('Loading "%s"' % filename)

        with open(filename, 'r') as f:
            lines = f.readlines()

        self.constructed = False
        for line in lines:
            pos = line.find('=')
            if pos < 1:
                continue
            key = line[:pos]
            if key not in self.fields:
                continue
            value = line[pos + 1:].strip()
            if isinstance(self.fields[key], QtWidgets.QLineEdit):
                self.fields[key].setText(value)
            elif isinstance(self.fields[key], QtWidgets.QSpinBox):
                self.fields[key].setValue(int(value))
            elif isinstance(self.fields[key], QtWidgets.QCheckBox):
                self.fields[key].setChecked(int(value))
            elif isinstance(self.fields[key], QtWidgets.QComboBox):
                index = \
                    int(getComboBoxIndexFromData(self.fields[key], str(value)))
                self.fields[key].setCurrentIndex(index)
            # make sure we only refresh when loading a new 'scenefile'.
            if key == 'scenefile':
                self.fileChange()
        self.constructed = True
        self.evaluate()
        return True

    def browseLoad(self):
        path, fltr = QtWidgets.QFileDialog.getOpenFileName(
                self,
                'Choose afstarter file',
                cgruconfig.VARS['HOME_CGRU']
            )
        if path == '': return
        self.load(path, True)

    def browseSave(self):
        path, fltr = QtWidgets.QFileDialog.getSaveFileName(
                self,
                'Choose afstarter file',
                cgruconfig.VARS['HOME_CGRU']
            )
        if path == '': return
        self.save(path, True)

    def evaluate(self):
        self.evaluateCmd()

    def evaluateCmd(self, i_start = False):
        if not self.constructed:
            return

        self.evaluated = False
        self.bStart.setEnabled(False)
        # self.fileChange()

        # Check parameters:
        # Frame range:

        framestart = self.fields['framestart'].value()
        frameend = self.fields['frameend'].value()
        frameby = self.fields['frameby'].value()
        framespt = self.fields['framespt'].value()

        if i_start:

            if framestart > frameend:
                QtWidgets.QMessageBox.critical(
                    self,
                    'Critical',
                    'First frame is greater that last.')
                return

            tasksnum = (1.0 + frameend - framestart) / (1.0 * frameby * framespt)

            if tasksnum > 10000.0:
                answer = QtWidgets.QMessageBox.warning(
                    self,
                    'Warning',
                    'Number of tasks > 10000',
                    QtWidgets.QMessageBox.Yes | QtWidgets.QMessageBox.Abort
                )
                if answer != QtWidgets.QMessageBox.Yes:
                    return
            if tasksnum > 100000.0:
                answer = QtWidgets.QMessageBox.warning(
                    self,
                    'Warning',
                    'Number of tasks > 100000',
                    QtWidgets.QMessageBox.Yes | QtWidgets.QMessageBox.Abort
                )
                if answer != QtWidgets.QMessageBox.Yes:
                    return
            if tasksnum > 1000000.0:
                self.teCmd.setText(
                    'The number of tasks over one million. Call you system '
                    'administrator, supervisor or TD.'
                )
                return

        # Check wdir:
        if self.fields['scenewdir'].isChecked():
            self.fields['wdir'].setEnabled(False)
        else:
            self.fields['wdir'].setEnabled(True)
        # Set job name:
        if self.fields['jobnamescene'].isChecked():
            self.fields['jobname'].setText(
                os.path.basename(str(self.fields['scenefile'].text()))
            )
            self.fields['jobname'].setEnabled(False)
        else:
            self.fields['jobname'].setEnabled(True)

        # Check scene:
        scene = str(self.fields['scenefile'].text())
        if len(scene) == 0:
            return

        name, ext = os.path.splitext(scene)
        if ext in ScenesSequences:
            scene = afcommon.patternFromFile(scene)
        elif not os.path.isfile(scene):
            self.teCmd.setText('Scene file does not exist.')
            return

        scene = os.path.normpath(scene)
        scene = os.path.abspath(scene)
        self.fields['scenefile'].setText(scene)

        # Check working directory:
        if self.fields['scenewdir'].isChecked():
            self.fields['wdir'].setText(
                os.path.dirname(str(self.fields['scenefile'].text()))
            )

        if not os.path.isdir(self.fields['wdir'].text()):
            self.teCmd.setText('Working directory does not exist.')
            return
        self.fields['wdir'].setText(
            os.path.abspath(str(self.fields['wdir'].text()))
        )

        # Check job name:
        if str(self.fields['jobname'].text()) == '':
            self.teCmd.setText('Job name is empty.')
            return

        # Get File Type Override
        servType = getComboBoxString(self.fields['servicetype'])

        # Construct command:
        cmd = os.environ['AF_ROOT']
        cmd = os.path.join(cmd, 'python')
        cmd = os.path.join(cmd, 'afjob.py')
        cmd = '"%s" "%s"' % (os.getenv('CGRU_PYTHONEXE', 'python'), cmd)
        cmd += ' "%s"' % self.fields['scenefile'].text()
        cmd += ' %d' % self.fields['framestart'].value()
        cmd += ' %d' % self.fields['frameend'].value()
        cmd += ' -by %d' % self.fields['frameby'].value()
        cmd += ' -fpt %d' % self.fields['framespt'].value()
        cmd += ' -seq %d' % self.fields['frameseq'].value()
        if not str(self.fields['node'].text()) == '':
            cmd += ' -node "%s"' % self.fields['node'].text()

        if not str(self.fields['take'].text()) == '':
            cmd += ' -take "%s"' % self.fields['take'].text()

        if not str(self.fields['outimages'].text()) == '':
            cmd += ' -output "%s"' % self.fields['outimages'].text()

        if not str(self.fields['preview'].text()) == '':
            cmd += ' -images "%s"' % self.fields['preview'].text()

        if not str(self.fields['extrargs'].text()) == '':
            cmd += ' -extrargs "%s"' % self.fields['extrargs'].text()

        if len(servType) and servType != 'none':
            cmd += ' -type "%s"' % servType

        if str(self.fields['os'].text()) == '':
            cmd += ' -os any'
        else:
            cmd += ' -os "%s"' % self.fields['os'].text()

        cmd += ' -pwd "%s"' % self.fields['wdir'].text()

        if self.fields['capacity'].value() > 0:
            cmd += ' -capacity %d' % self.fields['capacity'].value()

        if self.fields['maxruntasks'].value() > 0:
            cmd += ' -maxruntasks %d' % self.fields['maxruntasks'].value()

        if self.fields['maxtasksperhost'].value() > 0:
            cmd += ' -maxtasksperhost %d' % self.fields['maxtasksperhost'].value()

        if self.fields['priority'].value() > -1:
            cmd += ' -priority %d' % self.fields['priority'].value()

        if not str(self.fields['dependmask'].text()) == '':
            cmd += ' -depmask "%s"' % self.fields['dependmask'].text()

        if not str(self.fields['dependglobal'].text()) == '':
            cmd += ' -depglbl "%s"' % self.fields['dependglobal'].text()

        if not str(self.fields['hostsmask'].text()) == '':
            cmd += ' -hostsmask "%s"' % self.fields['hostsmask'].text()

        if not str(self.fields['hostsexclude'].text()) == '':
            cmd += ' -hostsexcl "%s"' % self.fields['hostsexclude'].text()

        if self.fields['paused'].isChecked():
            cmd += ' -pause'

        cmd += ' -name "%s"' % self.fields['jobname'].text()

        if not str(self.fields['exec'].text()) == '':
            cmd += ' -exec "%s"' % self.fields['exec'].text()

        # Evaluated:
        self.teCmd.setText(cmd)
        self.evaluated = True
        self.bStart.setEnabled(True)
        print('Evaluated')

    def start(self):
        self.evaluateCmd( True)
        if not self.evaluated:
            return

        self.bStart.setEnabled(False)
        command = str(self.teCmd.toPlainText())

        if len(command) == 0:
            return

        self.teCmd.clear()
        time.sleep(.1)
        self.process = QtCore.QProcess(self)
        self.process.setProcessChannelMode(QtCore.QProcess.MergedChannels)
        self.process.finished.connect( self.processfinished)
        self.process.readyRead.connect( self.processoutput)
        self.output = ''
        self.process.start(command)

    def processfinished(self, exitCode):
        print('Exit code = %d' % exitCode)
        if exitCode != 0:
            return
        self.saveRecent()
        self.save(FileLast)
        self.output += '\n The job successfully sent.'
        self.teCmd.setText(self.output)
        self.bStart.setEnabled(False)

    def processoutput(self):
        output = self.process.readAll()
        print(output.__class__.__name__)
        if sys.version_info[0] < 3:
            output = str(output)
        else:
            if output.__class__.__name__ == 'QByteArray':
                output = str(output.data(), 'utf-8')
            else:
                output = str(output, 'utf-8')
        print(output)
        self.teCmd.insertPlainText(output)
        self.teCmd.moveCursor(QtGui.QTextCursor.End)
        self.output += output


app = QtWidgets.QApplication(sys.argv)
dialog = Dialog()
dialog.show()
app.exec_()
