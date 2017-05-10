# -*- coding: utf-8 -*-
"""
	Softimage submitter 2.2
	original Author: Timur Hairulin

	continued and extended by: Stephan Haidacher
"""
import os

import win32com.client
from win32com.client import constants

# Get handles to the xsi application
Application = win32com.client.Dispatch('XSI.Application')
# XSIUtils = win32com.client.Dispatch('XSI.Utils')
# XSIUIToolkit = win32com.client.Dispatch('XSI.UIToolkit')

def GetOpSetValue( opSet, name, defaultValue ):
	if opSet.Parameters(name) != None:
		return opSet.Parameters(name).Value
	return defaultValue

def Main():
	print('Submit To Afanasy:')
	scene = Application.ActiveProject.ActiveScene

	defaultRange_frompass = 1
	defaultRange_forcepass = 0
	defaultFrame_start = 1
	defaultFrame_end = 1
	defaultFrame_by = 1
	defaultFrame_fpt = 1
	defaultJobName = scene.Name
	defaultJobNameAuto = 1
	defaultPriority = 50
	defaultSimulate = False
	defaultStartPaused = False
	defaultMaxHosts = -1
	defaultMaxRunTime = 0.0
	defaultCapacity = 1000
	defaultHostsMask = ''
	defaultHostsMaskExclude = ''
	defaultDependMask = ''
	defaultDependMaskGlobal = ''
	defaultVariRender = False
	defaultVariRenderAttr = ''
	defaultVariRenderStart = 0
	defaultVariRenderStep = 1
	defaultVariRenderCount = 4
	defaultArnoldWatermarked = 0
	defaultUseTemp = 1
	defaultTempScenePath = 'P:\\- RENDERTEMP -\\'
	defaultArnoldAutoThread = 1
	defaultSkipFrame = 0
	defaultArnoldThreadsBool = 0
	defaultArnoldThreadsCount = 4
	defaultBucket = 64
	defaultStillimage = False
	defaultProgressive = False
	defaultRayReserved = 0


	# Pass selection logic
	if Application.Selection.Count > 0:
		defaultRenderPass = '_selected_'
	else:
		defaultRenderPass = '_current_'
   
	passControlList = [ defaultRenderPass, defaultRenderPass]
	passControlList.extend(['_selected_','_selected_'])
	passControlList.extend(['_current_','_current_'])
	passControlList.extend(['_all_','_all_'])
	passCount = 3
	for currPass in scene.Passes:
		passCount += 1
		passControlList.append( currPass.Name)
		passControlList.append( currPass.Name)


	opSet = Application.ActiveSceneRoot.Properties('afSubmitProperties')

	if(opSet != None):
		defaultJobNameAuto 		= 		 GetOpSetValue( opSet, 'afJobNameAuto',			defaultJobNameAuto )
		if not defaultJobNameAuto:
			defaultJobName		  	=		 GetOpSetValue( opSet, 'afJobName',				defaultJobName )

		defaultRange_frompass		= bool(  GetOpSetValue( opSet, 'afRange_frompass',		defaultRange_frompass ))
		defaultRange_forcepass		= bool(  GetOpSetValue( opSet, 'afRange_forcepass',		defaultRange_forcepass ))
		defaultFrame_start			= int(   GetOpSetValue( opSet, 'afFrame_start',			defaultFrame_start ))
		defaultFrame_end			= int(   GetOpSetValue( opSet, 'afFrame_end',			defaultFrame_end ))
		defaultFrame_by				= int(   GetOpSetValue( opSet, 'afFrame_by',			defaultFrame_by ))
		defaultFrame_fpt			= int(   GetOpSetValue( opSet, 'afFrame_fpt',			defaultFrame_fpt ))
		defaultSimulate				= bool(  GetOpSetValue( opSet, 'afSimulate',			defaultSimulate ))
		defaultStartPaused			= bool(  GetOpSetValue( opSet, 'afStartPaused',			defaultStartPaused ))

		# only load the pass setting from the properties when _all_ was selected and saved.
		# when no pass is selected _currect_ is used, when there is a selected _selected_ will be used
		afAllSaved					=		 GetOpSetValue( opSet, 'afRenderPass',		defaultRenderPass )
		if (afAllSaved == '_all_'):
			defaultRenderPass 		= 		 afAllSaved

		defaultPriority		  		= int(   GetOpSetValue( opSet, 'afPriority',			defaultPriority ))
		defaultCapacity		  		= int(   GetOpSetValue( opSet, 'afCapacity',			defaultCapacity ))
		defaultMaxHosts		  		= int(   GetOpSetValue( opSet, 'afMaxHosts',			defaultMaxHosts ))
		defaultMaxRunTime		  	= float( GetOpSetValue( opSet, 'afMaxRunTime',			defaultMaxRunTime ))
		defaultHostsMask		  	=		 GetOpSetValue( opSet, 'afHostsMask',			defaultHostsMask )
		defaultHostsMaskExclude 	=		 GetOpSetValue( opSet, 'afHostsMaskExclude',	defaultHostsMaskExclude )
		defaultDependMask		  	=		 GetOpSetValue( opSet, 'afDependMask',			defaultDependMask )
		defaultDependMaskGlobal 	=		 GetOpSetValue( opSet, 'afDependMaskGlobal',	defaultDependMaskGlobal )
		defaultVariRender		  	= bool(  GetOpSetValue( opSet, 'afVariRender',			defaultVariRender ))
		defaultVariRenderAttr	  	=		 GetOpSetValue( opSet, 'afVariRenderAttr',		defaultVariRenderAttr )
		defaultVariRenderStart  	= int(   GetOpSetValue( opSet, 'afVariRenderStart',		defaultVariRenderStart ))
		defaultVariRenderStep	 	= int(   GetOpSetValue( opSet, 'afVariRenderStep',		defaultVariRenderStep ))
		defaultVariRenderCount  	= int(   GetOpSetValue( opSet, 'afVariRenderCount',		defaultVariRenderCount ))
		defaultArnoldWatermarked	= bool(  GetOpSetValue( opSet, 'afArnoldWater',		 	defaultArnoldWatermarked ))
		defaultTempScenePath	  	=		 GetOpSetValue( opSet, 'afTempScenePath',		defaultTempScenePath )
		defaultUseTemp				=		 GetOpSetValue( opSet, 'afUseTemp',				defaultUseTemp )
		defaultArnoldAutoThread	 	=		 GetOpSetValue( opSet, 'afArnoldAutoThread',	defaultArnoldAutoThread )
		defaultSkipFrame 			= 		 GetOpSetValue( opSet, 'afSkipFrame', 			defaultSkipFrame )
		defaultArnoldThreadsBool 	= 		 GetOpSetValue( opSet, 'afArnoldThreadsBool', 	defaultSkipFrame )
		defaultArnoldThreadsCount 	= int(	 GetOpSetValue( opSet, 'afArnoldThreadsCount',	defaultArnoldThreadsCount ))
		defaultBucket 				= int(	 GetOpSetValue( opSet, 'afBucket',				defaultBucket ))
		defaultStillimage 			= 		 GetOpSetValue( opSet, 'afStillimage', 			defaultStillimage )
		defaultProgressive 			= bool(  GetOpSetValue( opSet, 'afProgressive', 		defaultProgressive ))
		defaultRayReserved 			= int(	 GetOpSetValue( opSet, 'afRayReserved',			defaultRayReserved ))

		pSet = None
		Application.ExecuteCommand('DeleteObj',[str(Application.ActiveSceneRoot) + '.afSubmitProperties'])

	opSet = Application.ActiveSceneRoot.AddProperty('CustomProperty',False,'afSubmitProperties')
	opSet.AddParameter3('afJobNameAuto',		 constants.siBool,	  defaultJobNameAuto, 0, 1, False)
	opSet.AddParameter3('afJobName',			 constants.siString,  defaultJobName)
	opSet.AddParameter3('afRange_frompass',	 	 constants.siBool,	  defaultRange_frompass, 0, 1, False)
	opSet.AddParameter3('afRange_forcepass',	 constants.siBool,	  defaultRange_forcepass, 0, 1, False)
	opSet.AddParameter3('afFrame_start',		 constants.siInt4,	  defaultFrame_start, 1, 100000, False)
	opSet.AddParameter3('afFrame_end',		 	 constants.siInt4,	  defaultFrame_end, 1, 100000, False)
	opSet.AddParameter3('afFrame_by',		 	 constants.siInt4,	  defaultFrame_by, 1, 1000, False)
	opSet.AddParameter3('afFrame_fpt',		 	 constants.siInt4,	  defaultFrame_fpt, 1, 1000, False)
	opSet.AddParameter3('afRenderPass',		 	 constants.siString,  defaultRenderPass)
	opSet.AddParameter3('afSimulate',		 	 constants.siBool,	  defaultSimulate, 0, 1, False)
	opSet.AddParameter3('afStartPaused',		 constants.siBool,	  defaultStartPaused, 0, 1, False)
	opSet.AddParameter3('afPriority',		 	 constants.siInt2,	  defaultPriority, -1, 99, False)
	opSet.AddParameter3('afCapacity',		 	 constants.siInt2,	  defaultCapacity, -1, 10000, False)
	opSet.AddParameter3('afMaxHosts',		 	 constants.siInt2,	  defaultMaxHosts, -1, 10000, False)
	opSet.AddParameter3('afMaxRunTime',		 	 constants.siFloat,	  defaultMaxRunTime, 0,	 1000, False)
	opSet.AddParameter3('afHostsMask',		 	 constants.siString,  defaultHostsMask)
	opSet.AddParameter3('afHostsMaskExclude', 	 constants.siString,  defaultHostsMaskExclude)
	opSet.AddParameter3('afDependMask',		 	 constants.siString,  defaultDependMask)
	opSet.AddParameter3('afDependMaskGlobal', 	 constants.siString,  defaultDependMaskGlobal)
	opSet.AddParameter3('afVariRender',		 	 constants.siBool,	  defaultVariRender, 0, 1, False)
	opSet.AddParameter3('afVariRenderAttr',	 	 constants.siString,  defaultVariRenderAttr, -1000000, 1000000, False)
	opSet.AddParameter3('afVariRenderStart',	 constants.siInt4,	  defaultVariRenderStart, -1000000, 1000000, False)
	opSet.AddParameter3('afVariRenderStep',	 	 constants.siInt4,	  defaultVariRenderStep, -1000000, 1000000, False)
	opSet.AddParameter3('afVariRenderCount',	 constants.siInt4,	  defaultVariRenderCount, 2, 1000000, False)
	opSet.AddParameter3('afArnoldWater',		 constants.siBool,	  defaultArnoldWatermarked, 0, 1, False)
	opSet.AddParameter3('afTempScenePath',	 	 constants.siString,  defaultTempScenePath)
	opSet.AddParameter3('afArnoldAutoThread',	 constants.siBool,	  defaultArnoldAutoThread, 0, 1, False)
	opSet.AddParameter3('afUseTemp',	   		 constants.siBool,	  defaultUseTemp, 0, 1, False)
	opSet.AddParameter3('afSkipFrame',	 		 constants.siBool,	  defaultSkipFrame, 0, 1, False)
	opSet.AddParameter3('afArnoldThreadsBool',	 constants.siBool,	  defaultArnoldThreadsBool, 0, 1, False)
	opSet.AddParameter3('afArnoldThreadsCount',	 constants.siInt2,	  defaultArnoldThreadsCount, 1, 99, False)
	opSet.AddParameter3('afBucket',		   		 constants.siInt2,	  defaultBucket, 8, 512, False)
	opSet.AddParameter3('afStillimage',	  		 constants.siBool,	  defaultStillimage, 0, 1, False)
	opSet.AddParameter3('afProgressive',	 	 constants.siBool,	  defaultProgressive, 0, 1, False)
	opSet.AddParameter3('afRayReserved', 		 constants.siInt4, 	  defaultRayReserved, 0, 1000000, False)


# CREATE Layout -------------------------------------------------------
	oPPGLayout = opSet.PPGLayout
	oPPGLayout.Clear()
	oPPGLayout.SetViewSize(360,800)

# ------------------------------------
	oPPGLayout.AddTab('Basic Settings')
	oPPGLayout.AddSpacer(0,10)

# -------

	oPPGLayout.AddGroup('Submission',True,100)

	oPPGLayout.AddItem('afJobNameAuto', 'Auto Job Name = Scene Name', 	 constants.siControlBoolean)
	oPPGLayout.AddItem('afJobName',	    'Job Name',						 constants.siControlString)
	oPPGLayout.AddSpacer(0,12)

	oPPGLayout.AddEnumControl('afRenderPass', passControlList, 'Pass', 	 constants.siControlCombo)

	oPPGLayout.AddRow()
	oPPGLayout.AddItem('afPriority',			 'Priority',			 constants.siControlNumber)
	oPPGLayout.AddItem('afCapacity',			 'Capacity',			 constants.siControlNumber)
	oPPGLayout.AddItem('afMaxHosts',			 'MaxHosts',			 constants.siControlNumber)
	oPPGLayout.EndRow()

	oPPGLayout.AddSpacer(0,20)

	oPPGLayout.AddGroup('global Framerange',True,100)
	oPPGLayout.AddItem('afRange_forcepass', 	 'override',	  		 constants.siControlBoolean)
	oPPGLayout.AddItem('afFrame_start', 		 'Start Frame',			 constants.siControlNumber)
	oPPGLayout.AddItem('afFrame_end',   		 'End Frame',		 	 constants.siControlNumber)
	oPPGLayout.AddItem('afFrame_by',	   		 'Step',				 constants.siControlNumber)
	oPPGLayout.EndGroup()
	oPPGLayout.EndGroup()

# -------

	oPPGLayout.AddGroup('Options',True,100)
	oPPGLayout.AddItem('afBucket', 				 'Bucketsize',		  	 constants.siControlNumber)
	oPPGLayout.AddItem('afMaxRunTime',		 	 'max hours', 			 constants.siControlNumber)
	oPPGLayout.AddItem('afFrame_fpt',   		 'Task size',			 constants.siControlNumber)
	oPPGLayout.AddItem('afStartPaused', 		 'Start Job Paused', 	 constants.siControlBoolean)
	oPPGLayout.AddItem('afSkipFrame', 			 'Skip existing frames', constants.siControlBoolean)
	oPPGLayout.AddItem('afStillimage', 			 'tiled, no crop EXRs',  constants.siControlBoolean)
	oPPGLayout.EndGroup()

# -------

	oPPGLayout.AddGroup('Redshift specific',True,100)
	oPPGLayout.AddItem('afProgressive', 		 'Progressive rendering', constants.siControlBoolean)
	oPPGLayout.AddItem('afRayReserved', 		 'RayReservedMemory', constants.siControlNumber)
	oPPGLayout.EndGroup()

# -------

	oPPGLayout.AddGroup('Arnold specific',True,100)
	oPPGLayout.AddItem('afArnoldWater', 		 'Render Watermarked', 			constants.siControlBoolean)
	oPPGLayout.AddItem('afArnoldAutoThread', 	 'Force Thread Autodetection',  constants.siControlBoolean)
	oPPGLayout.AddItem('afArnoldThreadsBool', 	 'Limit Threads to:', 			constants.siControlBoolean)
	oPPGLayout.AddItem('afArnoldThreadsCount', 	 'max',	   						constants.siControlNumber)
	oPPGLayout.EndGroup()

# -------

	oPPGLayout.AddRow()
	oPPGLayout.AddButton('SubmitButton', 'Submit To Afanasy')
	oPPGLayout.AddButton('CloseButton',	 'Close Dialog')
	oPPGLayout.EndRow()


# ------------------------------------
	oPPGLayout.AddTab('Advanced')
	oPPGLayout.AddSpacer(0,10)

	oPPGLayout.AddGroup('Advanced Render Options',True,100)
	oPPGLayout.AddItem('afUseTemp', 	  'Use Temp Directory [or save all temp scenes in the current project]', constants.siControlBoolean)
	oPPGLayout.AddItem('afTempScenePath', 'Temp Dir',		    constants.siControlString)
	oPPGLayout.AddSpacer(0,10)

	oPPGLayout.AddItem('afSimulate', 		 'Simulate', 			constants.siControlBoolean)
	oPPGLayout.AddItem('afHostsMask',		 'Hosts Mask',			constants.siControlString)
	oPPGLayout.AddItem('afHostsMaskExclude', 'Exclude Hosts Mask',	constants.siControlString)
	oPPGLayout.AddItem('afDependMask',		 'Depend Mask',			constants.siControlString)
	oPPGLayout.AddItem('afDependMaskGlobal', 'Global Depend Mask',	constants.siControlString)
	oPPGLayout.EndGroup()

	# currently broken and disabled
	"""
	oPPGLayout.AddSpacer(0,20)

	oPPGLayout.AddGroup('Parameter Variation',True,100)
	oPPGLayout.AddItem('afVariRender',		 'Enable VariRender', constants.siControlBoolean)
	oPPGLayout.AddItem('afVariRenderAttr',	 'Arribute',		  constants.siControlString)
	oPPGLayout.AddItem('afVariRenderStart',	 'Start Value',		  constants.siControlNumber)
	oPPGLayout.AddItem('afVariRenderStep',	 'Step',			  constants.siControlNumber)
	oPPGLayout.AddItem('afVariRenderCount',	 'Count',			  constants.siControlNumber)
	oPPGLayout.EndGroup()
	"""


	oPPGLayout.AddButton('HelpButton',	'Show Help')

	oPPGLayout.Language = 'Python'
	cgru_xsi = os.getenv('XSI_CGRU_PATH')
	if cgru_xsi is None or cgru_xsi == '':
		Application.LogMessage('XSI_CGRU_PATH is not set, can`t find Afanasy plug-in.')
	else:
		logic_script = os.path.join(cgru_xsi, 'afxsisubmit_logic.py')
		if os.path.exists(logic_script):
			textStream = open(logic_script, 'r')
			logic = textStream.read()
			oPPGLayout.Logic = logic
			textStream.close()
		else:
			Application.LogMessage(
				'Script logic file not found:\n%s' % logic_script
			)

	oView = Application.Desktop.ActiveLayout.CreateView( 'Property Panel', 'Afanasy Submit' )
	oView.BeginEdit()
	oView.SetAttributeValue('targetcontent', opSet.FullName)
	oView.EndEdit()
