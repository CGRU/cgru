import os
import win32com.client
from win32com.client import constants
#Get handles to the xsi application
Application = win32com.client.Dispatch( 'XSI.Application' )
XSIUtils = win32com.client.Dispatch( 'XSI.Utils' )
XSIUIToolkit = win32com.client.Dispatch( 'XSI.UIToolkit' )

def GetOpSetValue( opSet, name, defaultValue ):
   if opSet.Parameters(name) != None:
      return opSet.Parameters(name).Value
   return defaultValue

def Main():
   print('Submit To Afanasy:')

   scene = Application.ActiveProject.ActiveScene

   defaultFrame_start = 1
   defaultFrame_end = 1
   defaultFrame_by = 1
   defaultFrame_fpt = 1
   defaultJobName = scene.Name
   defaultPriority = -1
   defaultStartPaused = False
   defaultMaxHosts = -1
   defaultCapacity = -1
   defaultHostsMask = ''
   defaultHostsMaskExclude = ''
   defaultDependMask = ''
   defaultDependMaskGlobal = ''

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
      defaultJobName          =        GetOpSetValue( opSet, 'afJobName',           defaultJobName )
      defaultFrame_start      = int(   GetOpSetValue( opSet, 'afFrame_start',       defaultFrame_start ))
      defaultFrame_end        = int(   GetOpSetValue( opSet, 'afFrame_end',         defaultFrame_end ))
      defaultFrame_by         = int(   GetOpSetValue( opSet, 'afFrame_by',          defaultFrame_by ))
      defaultFrame_fpt        = int(   GetOpSetValue( opSet, 'afFrame_fpt',         defaultFrame_fpt ))
      defaultStartPaused      = bool(  GetOpSetValue( opSet, 'afStartPaused',       defaultStartPaused ))
      defaultRenderPass       =        GetOpSetValue( opSet, 'afRenderPass',        defaultRenderPass )
      defaultPriority         = int(   GetOpSetValue( opSet, 'afPriority',          defaultPriority ))
      defaultCapacity         = int(   GetOpSetValue( opSet, 'afCapacity',          defaultCapacity ))
      defaultMaxHosts         = int(   GetOpSetValue( opSet, 'afMaxHosts',          defaultMaxHosts ))
      defaultHostsMask        =        GetOpSetValue( opSet, 'afHostsMask',         defaultHostsMask )
      defaultHostsMaskExclude =        GetOpSetValue( opSet, 'afHostsMaskExclude',  defaultHostsMaskExclude )
      defaultDependMask       =        GetOpSetValue( opSet, 'afDependMask',        defaultDependMask )
      defaultDependMaskGlobal =        GetOpSetValue( opSet, 'afDependMaskGlobal',  defaultDependMaskGlobal )
      opSet = None
      Application.ExecuteCommand('DeleteObj',[str(Application.ActiveSceneRoot) + '.afSubmitProperties'])

   opSet = Application.ActiveSceneRoot.AddProperty('CustomProperty',False,'afSubmitProperties')
   opSet.AddParameter3('afJobName',          constants.siString,  defaultJobName)
   opSet.AddParameter3('afFrame_start',      constants.siInt4,    defaultFrame_start, -1000000, 1000000, False)
   opSet.AddParameter3('afFrame_end',        constants.siInt4,    defaultFrame_end, -1000000, 1000000, False)
   opSet.AddParameter3('afFrame_by',         constants.siInt4,    defaultFrame_by, -1000000, 1000000, False)
   opSet.AddParameter3('afFrame_fpt',        constants.siInt4,    defaultFrame_fpt, -1000000, 1000000, False)
   opSet.AddParameter3('afRenderPass',       constants.siString,  defaultRenderPass)
   opSet.AddParameter3('afStartPaused',      constants.siBool,    defaultStartPaused, 0, 1, False)
   opSet.AddParameter3('afPriority',         constants.siInt2,    defaultPriority, -1, 99, False)
   opSet.AddParameter3('afCapacity',         constants.siInt2,    defaultCapacity, -1, 10000, False)
   opSet.AddParameter3('afMaxHosts',         constants.siInt2,    defaultMaxHosts, -1, 10000, False)
   opSet.AddParameter3('afHostsMask',        constants.siString,  defaultHostsMask)
   opSet.AddParameter3('afHostsMaskExclude', constants.siString,  defaultHostsMaskExclude)
   opSet.AddParameter3('afDependMask',       constants.siString,  defaultDependMask)
   opSet.AddParameter3('afDependMaskGlobal', constants.siString,  defaultDependMaskGlobal)

   oPPGLayout = opSet.PPGLayout
   oPPGLayout.Clear()

   oPPGLayout.AddTab('Submission Options')

   oPPGLayout.AddItem('afJobName',     'Job Name',          constants.siControlString)
   oPPGLayout.AddItem('afFrame_start', 'Start Frame',       constants.siControlNumber)
   oPPGLayout.AddItem('afFrame_end',   'End Frame',         constants.siControlNumber)
   oPPGLayout.AddItem('afFrame_by',    'By Frame',          constants.siControlNumber)
   oPPGLayout.AddItem('afFrame_fpt',   'Frames Per Task',   constants.siControlNumber)

   oPPGLayout.AddEnumControl('afRenderPass', passControlList, 'Pass', constants.siControlCombo)
   oPPGLayout.AddItem('afStartPaused', 'Start Job Paused', constants.siControlBoolean)

   oPPGLayout.AddGroup('Job Scheduling', True)
   oPPGLayout.AddItem('afPriority',          'Priority',             constants.siControlNumber)
   oPPGLayout.AddItem('afCapacity',          'Capacity',             constants.siControlNumber)
   oPPGLayout.AddItem('afMaxHosts',          'Max Hosts',            constants.siControlNumber)
   oPPGLayout.AddItem('afHostsMask',         'Hosts Mask',           constants.siControlString)
   oPPGLayout.AddItem('afHostsMaskExclude',  'Exclude Hosts Mask',   constants.siControlString)
   oPPGLayout.AddItem('afDependMask',        'Depend Mask',          constants.siControlString)
   oPPGLayout.AddItem('afDependMaskGlobal',  'Global Depend Mask',   constants.siControlString)
   oPPGLayout.EndGroup()

   oPPGLayout.AddRow()
   oPPGLayout.AddButton('SubmitButton', 'Submit To Afanasy')
   oPPGLayout.AddButton('CloseButton', 'Close Dialog')
#	oPPGLayout.AddButton('ResetButton', 'Close Dialog And Reset Options')
   oPPGLayout.EndRow()

   oPPGLayout.Language = 'Python'
   logic_script = 'C:\\cg\\tools\\cgru\\afanasy\\trunk\\plugins\\xsi\\afxsisubmit_logic.py'
   afroot = os.getenv('AF_ROOT')
   if afroot is None or afroot == '':
      Application.LogMessage('AF_ROOT is not set, can`t find Afanasy.')
   else:
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
