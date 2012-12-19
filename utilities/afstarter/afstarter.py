# -*- coding: utf-8 -*-
import os, sys, time

import cgrupyqt
from cgrupyqt import QtCore, QtGui

import cgruconfig
import cgruutils

import afcommon

from optparse import OptionParser
Parser = OptionParser(usage="usage: %prog [scene]", version="%prog 1.0")
Parser.add_option('-V', '--verbose',    action='store_true', dest='verbose', default=False, help='Verbose mode')
(Options, Args) = Parser.parse_args()

# Initializations:
Scene = ''
if len(Args) > 0: Scene = Args[0]
FilePrefix = 'afstarter.'
FileSuffix = '.txt'
FileLast = 'last'
FileRecent = 'recent'

def getComboBoxString( comboBox):
   data = comboBox.itemData( comboBox.currentIndex())
   if data is None: return ''
   if isinstance( data, str): return data
   if isinstance( data, unicode): return data
   return str( comboBox.itemData( comboBox.currentIndex()).toString())
   # return comboBox.itemData( comboBox.currentIndex()).toString()

def getComboBoxIndexFromData( comboBox, data):
   dataDict = dict()
   value = int()
   for i in range(comboBox.count()):
      #dataDict[str(comboBox.itemData( i).toString())] = int( i)
      dataDict[comboBox.itemData( i)] = int( i)
   if data in dataDict:
      value = int(dataDict[str(data)])
   if value is None: return 0
   if isinstance( value, int): return value
   return value

def fileTypeList():
   fileType_list = list()
   # Syntax:
   #    ( [ 'file_ext', 'file_ext' ], 'internal_name', 'UI Name' )
   fileType_list.append( ( ['shk'], 'shake', 'Shake') )
   fileType_list.append( ( ['blend'], 'blender', 'Blender') )
   fileType_list.append( ( ['nk'], 'nuke', 'Nuke') )
   fileType_list.append( ( ['hip'], 'houdini', 'Houdini') )
   fileType_list.append( ( ['ifd'], 'mantra', 'Mantra') )
   fileType_list.append( ( ['mb', 'ma'], 'maya', 'Maya') )
   fileType_list.append( ( ['mb', 'ma'], 'maya_mental', 'Mental Ray') )
   fileType_list.append( ( ['mb', 'ma'], 'maya_delight', '3Delight') )
   fileType_list.append( ( ['scn'], 'xsi', 'Softimage') )
   fileType_list.append( ( ['max'], 'max', '3dsmax') )
   fileType_list.append( ( ['aep'], 'afterfx', 'After Effects') )
   return fileType_list

def fileType_internalNameToIndex(internalName):
   index = int()
   filetypes = fileTypeList()
   for i in range(len(filetypes)):
      filetype = filetypes[i]
      filetypeName = filetype[1]
      filetypeUIName = filetype[2]
      if (filetypeName == internalName):
          index = i
   return index

def fileType_internalNameToUIName(internalName):
   uiName = str()
   filetypes = fileTypeList()
   for i in filetypes:
      filetypeName = i[1]
      filetypeUIName = i[2]
      if (filetypeName == internalName):
         uiName = filetypeUIName
   return uiName

def fileType_uiNameToInternalName(uiName):
   internalName = str()
   filetypes = fileTypeList()
   for i in filetypes:
      filetypeName = i[1]
      filetypeUIName = i[2]
      if (filetypeUIName == uiName):
         internalName = filetypeName
   return internalName

def labelNodeNameDict():
   labelNode_name = dict()
   labelNode_name['default'] = 'Node/Camera:'
   labelNode_name['houdini'] = 'ROP:'
   labelNode_name['nuke'] = 'Write Node:'
   labelNode_name['maya'] = 'Camera:'
   labelNode_name['maya_delight'] = 'Camera:'
   labelNode_name['maya_mental'] = 'Camera:'
   labelNode_name['3dsmax'] = 'Camera:'
   labelNode_name['afterfx'] = 'Composition:'
   return labelNode_name

def labelNodeTooltipsDict():
   labelNode_tooltip = dict()
   labelNode_tooltip['default'] = ('Houdini ROP\n'
                                   'Nuke write node\n'
                                   'Maya camera\n'
                                   '3DSMAX camera\n'
                                   'AfterFX composition')
   labelNode_tooltip['houdini'] = 'Houdini ROP'
   labelNode_tooltip['nuke'] = 'Nuke write node\nExample: \'Write1\''
   labelNode_tooltip['maya'] = 'Maya camera\nExample: \'persp\''
   labelNode_tooltip['maya_delight'] = 'Maya camera\nExample: \'persp\''
   labelNode_tooltip['maya_mental'] = 'Maya camera\nExample: \'persp\''
   labelNode_tooltip['3dsmax'] = '3DSMAX camera'
   labelNode_tooltip['afterfx'] = 'AfterFX composition\nExample: \'Comp 1\''
   return labelNode_tooltip

def labelTakeNameDict():
   labelTake_name = dict()
   labelTake_name['default'] = 'Take/Layer/Pass/Batch:'
   labelTake_name['houdini'] = 'Take:'
   labelTake_name['xsi'] = 'Pass:'
   labelTake_name['nuke'] = 'Not Used:'
   labelTake_name['maya'] = 'Layer:'
   labelTake_name['maya_delight'] = 'Render Pass:'
   labelTake_name['maya_mental'] = 'Layer:'
   labelTake_name['3dsmax'] = 'Batch:'
   labelTake_name['afterfx'] = 'render settings template:'
   return labelTake_name

def labelTakeTooltipsDict():
   labelTake_tooltip = dict()
   labelTake_tooltip['default'] = ('Houdini take\n'
                                   'SoftImage pass\n'
                                   'Maya layer\n'
                                   '3DFM render pass\n'
                                   '3DSMAX batch\n'
                                   'AfterFX render settings template')
   labelTake_tooltip['houdini'] = 'Houdini take'
   labelTake_tooltip['xsi'] = 'SoftImage pass'
   labelTake_tooltip['nuke'] = '- Not used -'
   labelTake_tooltip['maya'] = 'Maya render layer'
   labelTake_tooltip['maya_delight'] = '3Delight for Maya Render Pass'
   labelTake_tooltip['maya_mental'] = 'Maya render layer'
   labelTake_tooltip['3dsmax'] = '3DSMAX batch'
   labelTake_tooltip['afterfx'] = 'AfterFX render settings template\nExample: \'Best Settings\''
   return labelTake_tooltip

# Dialog class
class Dialog( QtGui.QWidget):
   def __init__( self):

      # Main Window
      QtGui.QWidget.__init__( self)
      self.constructed = False
      self.evaluated   = False
      self.output      = ''
      self.setWindowTitle('Afanasy Starter   CGRU ' + os.getenv('CGRU_VERSION', ''))
      self.fields = dict()
      self.labels = dict()

      # Get File Type dependant dicts
      self.labelNode_name = labelNodeNameDict()
      self.labelNode_tooltip = labelNodeTooltipsDict()
      self.labelTake_name = labelTakeNameDict()
      self.labelTake_tooltip = labelTakeTooltipsDict()

      # Top level layouts:
      topLayout = QtGui.QVBoxLayout( self)
      tabwidget = QtGui.QTabWidget( self)
      topLayout.addWidget( tabwidget)

      # General Tab:
      generalwidget = QtGui.QWidget( self)
      tabwidget.addTab( generalwidget,'Scene')
      generallayout = QtGui.QVBoxLayout( generalwidget)

      # Job Tab:
      jobwidget = QtGui.QWidget( self)
      tabwidget.addTab( jobwidget,'Job')
      joblayout = QtGui.QVBoxLayout( jobwidget)

      # Advanced Tab:
      auxwidget = QtGui.QWidget( self)
      tabwidget.addTab( auxwidget,'Advanced')
      advlayout = QtGui.QVBoxLayout( auxwidget)

      # Service Type:
      lFileType = QtGui.QHBoxLayout()
      generallayout.addLayout( lFileType)
      lFileType.addWidget( QtGui.QLabel('Override Service Type:', self))
      self.fields['servicetype'] = QtGui.QComboBox( self)
      lFileType.addWidget( self.fields['servicetype'] )
      QtCore.QObject.connect( self.fields['servicetype'],
                              QtCore.SIGNAL('currentIndexChanged(int)'),
                              self.setFileType)

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
      lImages.addWidget( QtGui.QLabel('Output Images:', self))
      self.fields['outimages'] = QtGui.QLineEdit( self)
      lImages.addWidget( self.fields['outimages'])
      QtCore.QObject.connect( self.fields['outimages'], QtCore.SIGNAL('editingFinished()'), self.evaluate)
      lImages.addWidget( QtGui.QLabel('Browse'))
      bBrowseOutImages = QtGui.QPushButton('File', self)
      lImages.addWidget( bBrowseOutImages)
      QtCore.QObject.connect( bBrowseOutImages, QtCore.SIGNAL('pressed()'), self.browseOutImages)
      bBrowseOutFolder = QtGui.QPushButton('Dir', self)
      lImages.addWidget( bBrowseOutFolder)
      QtCore.QObject.connect( bBrowseOutFolder, QtCore.SIGNAL('pressed()'), self.browseOutFolder)

      # Frames:
      lFrames = QtGui.QHBoxLayout()
      generallayout.addLayout( lFrames)
      lFrames.addWidget( QtGui.QLabel('Frames:', self))
      self.fields['framestart'] = QtGui.QSpinBox( self)
      self.fields['framestart'].setRange( -1000000000, 1000000000)
      self.fields['framestart'].setValue(1)
      QtCore.QObject.connect( self.fields['framestart'], QtCore.SIGNAL('editingFinished()'), self.evaluate)
      lFrames.addWidget( self.fields['framestart'])
      self.fields['frameend'] = QtGui.QSpinBox( self)
      self.fields['frameend'].setRange( -1000000000, 1000000000)
      self.fields['frameend'].setValue(1)
      QtCore.QObject.connect( self.fields['frameend'], QtCore.SIGNAL('editingFinished()'), self.evaluate)
      lFrames.addWidget( self.fields['frameend'])
      lFrames.addWidget( QtGui.QLabel('by', self))
      self.fields['frameby'] = QtGui.QSpinBox( self)
      lFrames.addWidget( self.fields['frameby'])
      QtCore.QObject.connect( self.fields['frameby'], QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.fields['frameby'].setRange( 1, 1000000000)
      lFrames.addWidget( QtGui.QLabel('per task', self))
      self.fields['framespt'] = QtGui.QSpinBox( self)
      lFrames.addWidget( self.fields['framespt'])
      QtCore.QObject.connect( self.fields['framespt'], QtCore.SIGNAL('editingFinished()'), self.evaluate)
      self.fields['framespt'].setRange( 1, 1000000000)


      # Node / Camera / Take:
      lNode = QtGui.QHBoxLayout()
      generallayout.addLayout( lNode)
      self.labels['node'] = QtGui.QLabel(self.labelNode_name['default'])
      lNode.addWidget( self.labels['node'] )
      self.labels['node'].setToolTip(self.labelNode_tooltip['default'])
      self.fields['node'] = QtGui.QLineEdit( self)
      lNode.addWidget( self.fields['node'])
      QtCore.QObject.connect( self.fields['node'],
                              QtCore.SIGNAL('textEdited(QString)'),
                              self.evaluate)
      self.labels['take'] = QtGui.QLabel(self.labelTake_name['default'])
      lNode.addWidget( self.labels['take'] )
      self.labels['take'].setToolTip(self.labelTake_tooltip['default'])
      self.fields['take'] = QtGui.QLineEdit( self)
      lNode.addWidget( self.fields['take'])
      QtCore.QObject.connect( self.fields['take'], QtCore.SIGNAL('textEdited(QString)'), self.evaluate)



      # Advanced:
      # OS Type:
      osLayout = QtGui.QHBoxLayout()
      advlayout.addLayout( osLayout)
      osLayout.addWidget( QtGui.QLabel('Operating System Type:'))
      self.fields['os'] = QtGui.QLineEdit( self)
      osLayout.addWidget( self.fields['os'])
      QtCore.QObject.connect( self.fields['os'],
                              QtCore.SIGNAL('textEdited(QString)'),
                              self.evaluate)

      # Tasks Command:
      exelayout = QtGui.QHBoxLayout()
      advlayout.addLayout( exelayout)
      label = QtGui.QLabel('Tasks Executable:')
      label.setToolTip('Override launching programm')
      exelayout.addWidget( label)
      self.fields['exec'] = QtGui.QLineEdit( self)
      exelayout.addWidget( self.fields['exec'])
      QtCore.QObject.connect( self.fields['exec'], QtCore.SIGNAL('textEdited(QString)'), self.evaluate)
      self.execBrowseButton = QtGui.QPushButton('Browse', self)
      exelayout.addWidget( self.execBrowseButton)
      QtCore.QObject.connect( self.execBrowseButton, QtCore.SIGNAL('pressed()'), self.browseExec)

      # Extra Arguments:
      exarglayout = QtGui.QHBoxLayout()
      advlayout.addLayout( exarglayout)
      exarglayout.addWidget( QtGui.QLabel('Extra Arguments:'))
      self.fields['extrargs'] = QtGui.QLineEdit( self)
      exarglayout.addWidget( self.fields['extrargs'])
      QtCore.QObject.connect( self.fields['extrargs'], QtCore.SIGNAL('textEdited(QString)'), self.evaluate)

      # Preview:
      prvlayout = QtGui.QHBoxLayout()
      advlayout.addLayout( prvlayout)
      prvlayout.addWidget( QtGui.QLabel('Preview:'))
      self.fields['preview'] = QtGui.QLineEdit( self)
      prvlayout.addWidget( self.fields['preview'])
      QtCore.QObject.connect( self.fields['preview'], QtCore.SIGNAL('textEdited(QString)'), self.evaluate)

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

      self.constructed = True

      # Set window icon:
      iconpath = cgruutils.getIconFileName('afanasy')
      if iconpath is not None: self.setWindowIcon( QtGui.QIcon( iconpath))

      # Refresh recent:
      self.refreshRecent()

      # Load last settings:
      if not self.load( FileLast):
         self.evaluate()

   def browseScene( self):
      scene = cgrupyqt.GetOpenFileName( self,'Choose a file', self.fields['scenefile'].text())
      if scene == '': return
      self.fields['scenefile'].setText( os.path.normpath( scene))
      self.fileChange()
      self.evaluate()

   def browseOutImages( self):
      path = cgrupyqt.GetSaveFileName( self,'Choose a file', self.fields['outimages'].text())
      if path == '': return
      self.fields['outimages'].setText( os.path.normpath( path))
      self.evaluate()

   def browseOutFolder( self):
      path = QtGui.QFileDialog.getExistingDirectory( self,'Choose a directory', self.fields['outimages'].text())
      if path == '': return
      self.fields['outimages'].setText( os.path.normpath( path))
      self.evaluate()

   def browseExec( self):
      path = cgrupyqt.GetOpenFileName( self,'Choose a file', self.fields['exec'].text())
      if path == '': return
      self.fields['exec'].setText( os.path.normpath( path))
      self.evaluate()

   def fileChange( self ):
      scene_filepath = str()
      try:
         scene_filepath = self.fields['scenefile'].text()
      except KeyError:
         return False

      if os.path.isfile(scene_filepath):
         realValue = getComboBoxString( self.fields['servicetype'] )

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

         theFileTypes.append( 'No Override' )
         theFileTypesData.append( 'none' )
         if len(enabledFileTypes) >= 1:
            for enabledFileType in enabledFileTypes:
               theFileTypes.append( fileType_internalNameToUIName(enabledFileType) )
               theFileTypesData.append( enabledFileType )

         self.fields['servicetype'].clear()
         for i in range(len(theFileTypes)):
            self.fields['servicetype'].addItem(theFileTypes[i],
                                    theFileTypesData[i])

         theFileTypeData = ''
         if (len(theFileTypesData) == 2) and \
                not ((len(theFileTypesData) == 0) or \
                    (len(theFileTypesData) == 1)):
            theFileTypeData = theFileTypesData[1]
         else:
            theFileTypeData = realValue

         # run file type dependant function
         self.fileTypeDependantText( theFileTypeData )
      else:
         return False
      return True

   def setFileType( self ):
      real_value = getComboBoxString( self.fields['servicetype'] )
      self.fileTypeDependantText( real_value )
      if self.evaluated:
         self.evaluate()
      return True

   def fileTypeDependantText( self, theFileTypeData ):
      labelKeyName = 'node'
      try:
         self.labels[labelKeyName].setText(self.labelNode_name[theFileTypeData])
      except KeyError:
         self.labels[labelKeyName].setText(self.labelNode_name['default'])
      try:
         self.labels[labelKeyName].setToolTip(self.labelNode_tooltip[theFileTypeData])
      except KeyError:
         self.labels[labelKeyName].setToolTip(self.labelNode_tooltip['default'])

      labelKeyName = 'take'
      try:
         self.labels[labelKeyName].setText(self.labelTake_name[theFileTypeData])
      except KeyError:
         self.labels[labelKeyName].setText(self.labelTake_name['default'])
      try:
         self.labels[labelKeyName].setToolTip(self.labelTake_tooltip[theFileTypeData])
      except KeyError:
         self.labels[labelKeyName].setToolTip(self.labelTake_tooltip['default'])

      return True

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
         elif isinstance( self.fields[key], QtGui.QComboBox):
            # value = str( self.fields[key].itemData( self.fields[key].currentIndex()).toString())
            value = str( self.fields[key].itemData( self.fields[key].currentIndex()))
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
#      QtCore.QObject.disconnect( self.cbRecent, QtCore.SIGNAL('currentIndexChanged(int)'), self.loadRecent)
      QtCore.QObject.disconnect( self.cbRecent, QtCore.SIGNAL(          'activated(int)'), self.loadRecent)
      self.cbRecent.clear()
      for afile in self.getRecentFilesList():
         if afile[ : len(FilePrefix)] == FilePrefix: afile = afile[ len(FilePrefix) :]
         if afile[ -len(FileSuffix) : ] == FileSuffix: afile = afile[ : -len(FileSuffix)]
         short = afile
         if short[ : len(FileRecent)] == FileRecent: short = short[ len(FileRecent) :]
         short = short[2:]
         if len(short) > 20: short = short[:10] + ' .. ' + short[-10:]
         self.cbRecent.addItem( short, afile)
#      QtCore.QObject.connect( self.cbRecent, QtCore.SIGNAL('currentIndexChanged(int)'), self.loadRecent)
      QtCore.QObject.connect( self.cbRecent, QtCore.SIGNAL(       'activated(int)'), self.loadRecent)

   def loadRecent( self):
      self.load( getComboBoxString( self.cbRecent))

   def load( self, filename, fullPath = False):
      if not fullPath: filename = os.path.join( cgruconfig.VARS['HOME_CGRU'], FilePrefix) + filename + FileSuffix
      if not os.path.isfile( filename): return False
      print('Loading "%s"' % filename)

      file = open( filename,'r')
      lines = file.readlines()
      file.close()
      self.constructed = False
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
         elif isinstance( self.fields[key], QtGui.QComboBox):
            index = int(getComboBoxIndexFromData( self.fields[key], str(value)))
            self.fields[key].setCurrentIndex( index)
         # make sure we only refresh when loading a new 'scenefile'.
         if key == 'scenefile':
            self.fileChange()
      self.constructed = True
      self.evaluate()
      return True

   def browseLoad( self):
      filename = cgrupyqt.GetOpenFileName( self,'Choose afstarter file', cgruconfig.VARS['HOME_CGRU'])
      if filename == '': return
      self.load( filename, True)

   def browseSave( self):
      filename = cgrupyqt.GetSaveFileName( self,'Choose afstarter file', cgruconfig.VARS['HOME_CGRU'])
      if filename == '': return
      self.save( filename, True)

   def evaluate( self):
      if not self.constructed: return
      self.evaluated = False
      self.bStart.setEnabled( False)
      # self.fileChange()

      # Check parameters:
      # Frame range:
      if self.fields['framestart'].value() > self.fields['frameend'].value(): self.fields['frameend'].setValue( self.fields['framestart'].value())
      tasksnum = (1.0 + self.fields['frameend'].value() - self.fields['framestart'].value()) / (1.0 * self.fields['frameby'].value() * self.fields['framespt'].value())
      if tasksnum > 10000.0:
         if QtGui.QMessageBox.warning( self, 'Warning', 'Number of tasks > 10000', QtGui.QMessageBox.Yes | QtGui.QMessageBox.Abort) != QtGui.QMessageBox.Yes:
            return
      if tasksnum > 100000.0:
         if QtGui.QMessageBox.warning( self, 'Warning', 'Number of tasks > 100000', QtGui.QMessageBox.Yes | QtGui.QMessageBox.Abort) != QtGui.QMessageBox.Yes:
            return
      if tasksnum > 1000000.0:
         self.teCmd.setText('The number of tasks over one million. Call you system administrator, supervisor or TD.')
         return
      # Check wdir:
      if self.fields['scenewdir'].isChecked(): self.fields['wdir'].setEnabled( False)
      else: self.fields['wdir'].setEnabled( True)
      # Set job name:
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

      # Get File Type Override
      servType = getComboBoxString( self.fields['servicetype'] )

      # Construct command:
      cmd = os.environ['AF_ROOT']
      cmd = os.path.join( cmd, 'python')
      cmd = os.path.join( cmd, 'afjob.py')
      cmd = '"%s" "%s"' % ( os.getenv('CGRU_PYTHONEXE','python'), cmd)
      cmd += ' "%s"' % self.fields['scenefile'].text()
      cmd += ' %d' % self.fields['framestart'].value()
      cmd += ' %d' % self.fields['frameend'].value()
      cmd += ' -by %d' % self.fields['frameby'].value()
      cmd += ' -fpt %d' % self.fields['framespt'].value()
      if not str( self.fields['node'].text()) == '': cmd += ' -node "%s"' % self.fields['node'].text()
      if not str( self.fields['take'].text()) == '': cmd += ' -take "%s"' % self.fields['take'].text()
      if not str( self.fields['outimages'].text()) == '': cmd += ' -output "%s"' % self.fields['outimages'].text()
      if not str( self.fields['preview'].text()) == '': cmd += ' -images "%s"' % self.fields['preview'].text()
      if not str( self.fields['extrargs'].text()) == '': cmd += ' -extrargs "%s"' % self.fields['extrargs'].text()
      if servType != 'none': cmd += ' -type "%s"' % servType
      if str( self.fields['os'].text()) == '': cmd += ' -os any'
      else: cmd += ' -os "%s"' % self.fields['os'].text()
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
      if not str( self.fields['exec'].text()) == '': cmd += ' -exec "%s"' % self.fields['exec'].text()

      # Evaluated:
      self.teCmd.setText( cmd)
      self.evaluated = True
      self.bStart.setEnabled( True)
      print('Evaluated')

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
      self.output = ''
      self.process.start( command)

   def processfinished( self, exitCode):
      print('Exit code = %d' % exitCode)
      if exitCode != 0: return
      self.saveRecent()
      self.save( FileLast)
      self.output += '\n The job successfully sent.'
      self.teCmd.setText( self.output)
      self.bStart.setEnabled( False)

   def processoutput( self):
      output = self.process.readAll()
      if sys.version_info[0] < 3:
         output = str( output)
      else:
         output = str( output, 'utf-8')
      print( output)
      self.teCmd.insertPlainText( output)
      self.teCmd.moveCursor( QtGui.QTextCursor.End)
      self.output += output

app = QtGui.QApplication( sys.argv)
dialog = Dialog()
dialog.show()
app.exec_()
