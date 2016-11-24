#
# Afanasy Group
#

import NatronEngine

def getPluginID():
	return 'cgru.afanasy'

def getLabel():
	return 'Afanasy'

def getVersion():
	return 207

def getGrouping():
	return ''

def createInstance(i_app,i_grp):

	# Create nodes in the group
	inputNode  = i_app.createNode('fr.inria.built-in.Input',  1, i_grp)
	outputNode = i_app.createNode('fr.inria.built-in.Output', 1, i_grp)
	outputNode.connectInput(0, inputNode)

	for i in range(4):
		i_app.createNode('fr.inria.built-in.Input', 1, i_grp)

	#
	# Multi-Write
	page = i_grp.createPageParam('multi_write','Multi-Write')

	p = i_grp.createBooleanParam('af_multi_independed','Connected Nodes Are Independent')
	p.setHelp('Render all connected write nodes independently.')
	p.setDefaultValue( False)
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	p = i_grp.createBooleanParam('af_multi_wholerange','Wait Whole Frame Range')
	p.setHelp('Down stream nodes will wait for the whole frame range to be rendered.')
	p.setDefaultValue( False)
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	p = i_grp.createBooleanParam('af_multi_forcerange','Force Upstream Frame Range')
	p.setHelp('All upsteam wtrite nodes will be rendered with current frame setting.')
	p.setDefaultValue( False)
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	#
	# Scheduling Page
	page = i_grp.createPageParam('scheduling','Scheduling')

	p = i_grp.createStringParam('af_platform','Platform')
	p.setHelp('If empty, any afrender plafrom can take tasks.\nDefault plaftorms are: "win","lin","mac".\nYou can check concrete value in Keeper.\nYou can configure any number any platfrom names.')
	p.setDefaultValue('')
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	p = i_grp.createIntParam('af_capacity','Capacity')
	p.setHelp('Tasks capacity.\n-1: Default value will be used.\nFor example, render with 2000 capacity can run 1x1000 task and 2x500 tasks at once.')
	p.setDefaultValue( -1, 0)
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	p = i_grp.createIntParam('af_maxtasks','Max Running Tasks')
	p.setHelp('Maximum tasks that job can run at once.\n-1: No limit.')
	p.setDefaultValue( -1, 0)
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	p = i_grp.createIntParam('af_maxtasks_perhost','Max Tasks Per Host')
	p.setHelp('Maximum tasks that job can run at once on the same host.\n-1: No limit.')
	p.setDefaultValue( -1, 0)
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	p = i_grp.createIntParam('af_priority','Priority')
	p.setHelp('-1: Default value will be used.')
	p.setDefaultValue( -1, 0)
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	p = i_grp.createStringParam('af_hostsmask','Hosts Mask')
	p.setHelp('Regular expression to match hostname to run a task.')
	p.setDefaultValue('')
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	p = i_grp.createStringParam('af_hostsmask_exclude','Exclude Hosts Mask')
	p.setHelp('Regular expression to match hostname not to run tasks.')
	p.setDefaultValue('')
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	p = i_grp.createStringParam('af_dependmask','Depend Mask')
	p.setHelp('Regular expression for same user job name(s) to wait to complete.')
	p.setDefaultValue('')
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	p = i_grp.createStringParam('af_dependmask_global','Global Depend Mask')
	p.setHelp('Regular expression for any user job name(s) to wait to complete.')
	p.setDefaultValue('')
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	#
	# General Page
	page = i_grp.createPageParam('general','General')

	p = i_grp.createStringParam('af_job_name','Afanasy Job Name')
	p.setHelp('If empty, project name will be used.')
	p.setDefaultValue('')
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	p = i_grp.createButtonParam('afanasy_button_frame_range','Get Frame Range From Project Settigs')
	p.setHelp('Set first and last frame parameters from current project settings.')
	p.setAddNewLine(True)
	p.setPersistent(False)
	p.setEvaluateOnChange(False)
	page.addParam(p)

	p = i_grp.createIntParam('af_frame_first','First Frame')
	p.setDefaultValue( i_app.timelineGetLeftBound(), 0)
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	p = i_grp.createIntParam('af_frame_last','Last Frame')
	p.setDefaultValue( i_app.timelineGetRightBound(), 0)
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	p = i_grp.createIntParam('af_frame_increment','Increment')
	p.setDefaultValue( 1, 0)
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	p = i_grp.createIntParam('af_frame_pertast','Frames Per Task')
	p.setDefaultValue( 1, 0)
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	p = i_grp.createIntParam('af_frame_sequential','Sequential')
	p.setDefaultValue( 1, 0)
	p.setHelp('Tasks solving method:\n1: Task by task.\n2: Each second frame first, than others.\n-1: The same, but backwards.\n0: Middle task.')
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(True)
	page.addParam(p)

	p = i_grp.createButtonParam('afanasy_button_send_job','Send Afanasy Job')
	p.setHelp('Constuct and send job to Afanasy server.')
	p.setAddNewLine(True)
	p.setPersistent(False)
	p.setEvaluateOnChange(False)
	page.addParam(p)

	p = i_grp.createBooleanParam('af_job_paused','Start Job Paused')
	p.setHelp('Set offline state to the job.')
	p.setDefaultValue( False)
	p.setEvaluateOnChange(False)
	p.setAnimationEnabled(False)
	p.setAddNewLine(False)
	page.addParam(p)

	'''
	p = i_grp.createButtonParam('add_input','Add Input')
	p.setAddNewLine(True)
	p.setPersistent(False)
	p.setEvaluateOnChange(False)
	page.addParam(p)
	'''

	#
	# Refresh the GUI with the newly created parameters
	i_grp.refreshUserParamsGUI()

	# Add callbacks
	i_grp.onInputChanged.set('afanasy.onInputChanged')
	i_grp.onParamChanged.set('afanasy.onParamChanged')

