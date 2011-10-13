import os

import win32com.client
from win32com.client import constants

#Get handles to the xsi application
Application = win32com.client.Dispatch('XSI.Application')
#XSIUtils = win32com.client.Dispatch('XSI.Utils')
#XSIUIToolkit = win32com.client.Dispatch('XSI.UIToolkit')

def GetOpSetValue( opSet, name, defaultValue ):
   if opSet.Parameters(name) != None:
      return opSet.Parameters(name).Value
   return defaultValue

def Main():
   print('Submit To Afanasy:')

   scene = Application.ActiveProject.ActiveScene

   defaultRange_frompass = 0
   defaultRange_forcepass = 0
   defaultFrame_start = 1
   defaultFrame_end = 1
   defaultFrame_by = 1
   defaultFrame_fpt = 1
   defaultJobName = scene.Name
   defaultJobNameAuto = 1
   defaultPriority = -1
   defaultSimulate = False
   defaultStartPaused = False
   defaultMaxHosts = -1
   defaultMaxRunTime = 0.0
   defaultCapacity = -1
   defaultHostsMask = ''
   defaultHostsMaskExclude = ''
   defaultDependMask = ''
   defaultDependMaskGlobal = ''
   defaultVariRender = False
   defaultVariRenderAttr = ''
   defaultVariRenderStart = 0
   defaultVariRenderStep = 1
   defaultVariRenderCount = 4

   defaultRenderPass = '_all_'
   passControlList = [ defaultRenderPass, defaultRenderPass]
   passControlList.extend(['_selected_','_selected_'])
   passControlList.extend(['_current_','_current_'])
   passCount = 3
   for currPass in scene.Passes:
      passCount += 1
      passControlList.append( currPass.Name)
      passControlList.append( currPass.Name)

   opSet = Application.ActiveSceneRoot.Properties('afSubmitProperties')
   if(opSet != None):
      defaultJobNameAuto      =        GetOpSetValue( opSet, 'afJobNameAuto',       defaultJobNameAuto )
      if not defaultJobNameAuto:
         defaultJobName       =        GetOpSetValue( opSet, 'afJobName',           defaultJobName )
      defaultRange_frompass   = bool(  GetOpSetValue( opSet, 'afRange_frompass',    defaultRange_frompass ))
      defaultRange_forcepass  = bool(  GetOpSetValue( opSet, 'afRange_forcepass',   defaultRange_forcepass ))
      defaultFrame_start      = int(   GetOpSetValue( opSet, 'afFrame_start',       defaultFrame_start ))
      defaultFrame_end        = int(   GetOpSetValue( opSet, 'afFrame_end',         defaultFrame_end ))
      defaultFrame_by         = int(   GetOpSetValue( opSet, 'afFrame_by',          defaultFrame_by ))
      defaultFrame_fpt        = int(   GetOpSetValue( opSet, 'afFrame_fpt',         defaultFrame_fpt ))
      defaultSimulate         = bool(  GetOpSetValue( opSet, 'afSimulate',          defaultSimulate ))
      defaultStartPaused      = bool(  GetOpSetValue( opSet, 'afStartPaused',       defaultStartPaused ))
      defaultRenderPass       =        GetOpSetValue( opSet, 'afRenderPass',        defaultRenderPass )
      defaultPriority         = int(   GetOpSetValue( opSet, 'afPriority',          defaultPriority ))
      defaultCapacity         = int(   GetOpSetValue( opSet, 'afCapacity',          defaultCapacity ))
      defaultMaxHosts         = int(   GetOpSetValue( opSet, 'afMaxHosts',          defaultMaxHosts ))
      defaultMaxRunTime       = float( GetOpSetValue( opSet, 'afMaxRunTime',        defaultMaxRunTime ))
      defaultHostsMask        =        GetOpSetValue( opSet, 'afHostsMask',         defaultHostsMask )
      defaultHostsMaskExclude =        GetOpSetValue( opSet, 'afHostsMaskExclude',  defaultHostsMaskExclude )
      defaultDependMask       =        GetOpSetValue( opSet, 'afDependMask',        defaultDependMask )
      defaultDependMaskGlobal =        GetOpSetValue( opSet, 'afDependMaskGlobal',  defaultDependMaskGlobal )
      defaultVariRender       = bool(  GetOpSetValue( opSet, 'afVariRender',        defaultVariRender ))
      defaultVariRenderAttr   =        GetOpSetValue( opSet, 'afVariRenderAttr',    defaultVariRenderAttr )
      defaultVariRenderStart  = int(   GetOpSetValue( opSet, 'afVariRenderStart',   defaultVariRenderStart ))
      defaultVariRenderStep   = int(   GetOpSetValue( opSet, 'afVariRenderStep',    defaultVariRenderStep ))
      defaultVariRenderCount  = int(   GetOpSetValue( opSet, 'afVariRenderCount',   defaultVariRenderCount ))

      opSet = None
      Application.ExecuteCommand('DeleteObj',[str(Application.ActiveSceneRoot) + '.afSubmitProperties'])

   opSet = Application.ActiveSceneRoot.AddProperty('CustomProperty',False,'afSubmitProperties')
   opSet.AddParameter3('afJobNameAuto',      constants.siBool,    defaultJobNameAuto, 0, 1, False)
   opSet.AddParameter3('afJobName',          constants.siString,  defaultJobName)
   opSet.AddParameter3('afRange_frompass',   constants.siBool,    defaultRange_frompass, 0, 1, False)
   opSet.AddParameter3('afRange_forcepass',  constants.siBool,    defaultRange_forcepass, 0, 1, False)
   opSet.AddParameter3('afFrame_start',      constants.siInt4,    defaultFrame_start, -1000000, 1000000, False)
   opSet.AddParameter3('afFrame_end',        constants.siInt4,    defaultFrame_end, -1000000, 1000000, False)
   opSet.AddParameter3('afFrame_by',         constants.siInt4,    defaultFrame_by, -1000000, 1000000, False)
   opSet.AddParameter3('afFrame_fpt',        constants.siInt4,    defaultFrame_fpt, -1000000, 1000000, False)
   opSet.AddParameter3('afRenderPass',       constants.siString,  defaultRenderPass)
   opSet.AddParameter3('afSimulate',         constants.siBool,    defaultSimulate, 0, 1, False)
   opSet.AddParameter3('afStartPaused',      constants.siBool,    defaultStartPaused, 0, 1, False)
   opSet.AddParameter3('afPriority',         constants.siInt2,    defaultPriority, -1, 99, False)
   opSet.AddParameter3('afCapacity',         constants.siInt2,    defaultCapacity, -1, 10000, False)
   opSet.AddParameter3('afMaxHosts',         constants.siInt2,    defaultMaxHosts, -1, 10000, False)
   opSet.AddParameter3('afMaxRunTime',       constants.siFloat,   defaultMaxRunTime, 0,  1000, False)
   opSet.AddParameter3('afHostsMask',        constants.siString,  defaultHostsMask)
   opSet.AddParameter3('afHostsMaskExclude', constants.siString,  defaultHostsMaskExclude)
   opSet.AddParameter3('afDependMask',       constants.siString,  defaultDependMask)
   opSet.AddParameter3('afDependMaskGlobal', constants.siString,  defaultDependMaskGlobal)
   opSet.AddParameter3('afVariRender',       constants.siBool,    defaultVariRender, 0, 1, False)
   opSet.AddParameter3('afVariRenderAttr',   constants.siString,  defaultVariRenderAttr, -1000000, 1000000, False)
   opSet.AddParameter3('afVariRenderStart',  constants.siInt4,    defaultVariRenderStart, -1000000, 1000000, False)
   opSet.AddParameter3('afVariRenderStep',   constants.siInt4,    defaultVariRenderStep, -1000000, 1000000, False)
   opSet.AddParameter3('afVariRenderCount',  constants.siInt4,    defaultVariRenderCount, 2, 1000000, False)

   oPPGLayout = opSet.PPGLayout
   oPPGLayout.Clear()

   oPPGLayout.AddTab('Submission')

   oPPGLayout.AddItem('afJobName',     'Job Name',          constants.siControlString)
   oPPGLayout.AddItem('afJobNameAuto', 'Auto Job Name = Scene Name', constants.siControlBoolean)

   oPPGLayout.AddEnumControl('afRenderPass', passControlList, 'Pass', constants.siControlCombo)

   oPPGLayout.AddItem('afRange_frompass',  'Take Frame Range From Pass', constants.siControlBoolean)
   oPPGLayout.AddItem('afRange_forcepass', 'Force Pass Frame Range',     constants.siControlBoolean)

   oPPGLayout.AddItem('afFrame_start', 'Start Frame',       constants.siControlNumber)
   oPPGLayout.AddItem('afFrame_end',   'End Frame',         constants.siControlNumber)
   oPPGLayout.AddItem('afFrame_by',    'By Frame',          constants.siControlNumber)
   oPPGLayout.AddItem('afFrame_fpt',   'Frames Per Task',   constants.siControlNumber)

   oPPGLayout.AddItem('afSimulate', 'Simulate', constants.siControlBoolean)

   oPPGLayout.AddRow()
   oPPGLayout.AddButton('SubmitButton', 'Submit To Afanasy')
   oPPGLayout.AddButton('HelpButton',   'Show Help')
   oPPGLayout.AddButton('CloseButton',  'Close Dialog')
   oPPGLayout.EndRow()

   oPPGLayout.AddItem('afStartPaused', 'Start Job Paused', constants.siControlBoolean)

   oPPGLayout.AddTab('Scheduling')
   oPPGLayout.AddItem('afPriority',          'Priority',             constants.siControlNumber)
   oPPGLayout.AddItem('afCapacity',          'Capacity',             constants.siControlNumber)
   oPPGLayout.AddItem('afMaxHosts',          'Max Hosts',            constants.siControlNumber)
   oPPGLayout.AddItem('afMaxRunTime',        'Task Max Run Time (hours)', constants.siControlNumber)
   oPPGLayout.AddItem('afHostsMask',         'Hosts Mask',           constants.siControlString)
   oPPGLayout.AddItem('afHostsMaskExclude',  'Exclude Hosts Mask',   constants.siControlString)
   oPPGLayout.AddItem('afDependMask',        'Depend Mask',          constants.siControlString)
   oPPGLayout.AddItem('afDependMaskGlobal',  'Global Depend Mask',   constants.siControlString)

   oPPGLayout.AddTab('VariRender')
   oPPGLayout.AddItem('afVariRender',        'Enable VariRender', constants.siControlBoolean)
   oPPGLayout.AddItem('afVariRenderAttr',    'Arribute',          constants.siControlString)
   oPPGLayout.AddItem('afVariRenderStart',   'Start Value',       constants.siControlNumber)
   oPPGLayout.AddItem('afVariRenderStep',    'Step',              constants.siControlNumber)
   oPPGLayout.AddItem('afVariRenderCount',   'Count',             constants.siControlNumber)

   oPPGLayout.Language = 'Python'
   afroot = os.getenv('AF_ROOT')
   if afroot is None or afroot == '':
      Application.LogMessage('AF_ROOT is not set, can`t find Afanasy.')
   else:
      logic_script = afroot
      logic_script = os.path.join( logic_script, 'plugins')
      logic_script = os.path.join( logic_script, 'xsi')
      logic_script = os.path.join( logic_script, 'afxsisubmit_logic.py')
      if( os.path.exists( logic_script)):
         textStream = open( logic_script,'r')
         logic = textStream.read()
         oPPGLayout.Logic = logic
         textStream.close()
      else:
         Application.LogMessage('Script logic file not founded:\n' + logic_script)

   oView = Application.Desktop.ActiveLayout.CreateView( 'Property Panel', 'AfanasyProperties' )
   oView.BeginEdit()
   oView.SetAttributeValue('targetcontent', opSet.FullName)
   oView.EndEdit()
