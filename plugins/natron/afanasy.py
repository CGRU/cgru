#
# Afanasy related tools
#

import os
import time

# Natron:
import NatronEngine

# Just to reload Afanasy group before each creation:
import afanasyGroup

# Afanasy Python API:
import af
import afcommon


if not NatronEngine.natron.isBackground():
	# For messages/warnings on job submission (gui mode only):
	NatronGui = __import__('NatronGui', globals(), locals())


def createNode( app):
	reload( afanasyGroup)
	app.createNode('cgru.afanasy')


def onParamChanged( thisParam, thisNode, thisGroup, app, userEdited):

	#print( thisNode.getLabel(), thisParam.getScriptName())

	if thisParam.getScriptName() == 'afanasy_button_send_job':
		renderNodes( app, [thisNode])
	elif thisParam.getScriptName() == 'afanasy_button_frame_range':
		thisNode.getParam('af_frame_first').setValue( app.timelineGetLeftBound())
		thisNode.getParam('af_frame_last' ).setValue( app.timelineGetRightBound())


def renderNodes( i_app, i_nodes, i_params = None):

	afparams_array = []

	for node in i_nodes:

		afparams = None

		if isNodeAfanasy( node):
			afparams = getAfParams( i_app, node, i_params)
		elif isNodeWrite( node):
			afparams = i_params
			if not 'af_frame_first' in afparams:
				afparams['af_frame_first'] = i_app.timelineGetLeftBound()
			if not 'af_frame_last' in afparams:
				afparams['af_frame_last'] = i_app.timelineGetRightBound()
			afparams['nodelabel'] = node.getLabel()
			wparams = getWriteParams( i_app, afparams, node)
			if wparams is None: return
			afparams['childs'] = [wparams]

		if afparams is None: return

		afparams_array.append( afparams)

	results = []

	for afparams in afparams_array:

		res = createJob( i_app, afparams)
		if res is None: return
		results.append( res)

	for res in results:
		res['job'].output()

		if not res['job'].send()[0]:
			NatronGui.natron.errorDialog('Error','Can`t send job to Afanasy server.\nSee script editor for details.')
			break

		i_app.saveTempProject( res['prj'])


def getAfParams( i_app, i_node, i_params):

	o_params = dict()

	# Get parameters from Afanasy node:
	for par in i_node.getParams():
		name = par.getScriptName()
		if name.find('af_') != 0: continue
		o_params[name] = par.getValue()

	# Override parametes when from dialog(F11) or when force frame range checked:
	if i_params:
		for par in i_params:
			o_params[par] = i_params[par]

	if len(o_params) == 0:
		NatronGui.natron.errorDialog('Error','No Afanasy parameters found on "%s"' % i_node.getLabel())
		return None

	o_params['afanasy'] = True
	o_params['nodelabel'] = i_node.getLabel()

	childs = getInputs( i_node)
	if len(childs) == 0:
		NatronGui.natron.errorDialog('Error','No valid connections found on "%s"' % i_node.getLabel())
		return None

	o_params['childs'] = []

	if o_params['af_multi_forcerange']:
		if i_params is None: i_params = dict()
		i_params['af_frame_first'] = o_params['af_frame_first']
		i_params['af_frame_last']  = o_params['af_frame_last']

	for child in childs:
		params = None
		if isNodeWrite( child):
			params = getWriteParams( i_app, o_params, child)
			if params is None: return None
		if isNodeAfanasy( child):
			params = getAfParams( i_app, child, i_params)
			if params is None: return None
		o_params['childs'].append( params)

	# Needed for depend masks, as first block should be most dependend
	o_params['childs'].reverse()

	return o_params


def getWriteParams( i_app, i_afparams, i_wnode):
	o_params = dict()

	pnames = ['filename']
	for pname in pnames:
		par = i_wnode.getParam(pname)
		if par:
			o_params[pname] = par.getValue()

	if len(o_params) == 0:
		NatronGui.natron.warningDialog('Error','No "filename" parameter found on "%s"' % i_wnode.getLabel())
		return None

	o_params['afanasy'] = False
	o_params['nodelabel'] = i_wnode.getLabel()
	o_params['nodename'] = i_wnode.getScriptName()

	files = o_params['filename']
	files = afcommon.patternFromStdC(files)
	files = afcommon.patternFromDigits(files)
	files = replaceProjectPaths( i_app, files)
	o_params['files'] = [files]

	return o_params


def createJob( i_app, i_params):
	# Construct job name:
	name = ''
	if 'af_job_name' in i_params: name = i_params['af_job_name']
	ext = '.ntp'
	if name == '':
		name, ext = os.path.splitext( i_app.getProjectParam('projectName').getValue())
		name += '.' + i_params['nodelabel']
	job = af.Job( name)

	# Generate temp project filename:
	prj = name
	prj += '.' + str(time.strftime('%y%M%d%H%M%S')) + str((time.time() - int(time.time())))[2:5]
	prj += ext
	prj = os.path.join( i_app.getProjectParam('projectPath').getValue(), prj)

	# Temp project should be deleteed at job deletion:
	job.setCmdPost('deletefiles "%s"' % prj)

	# Set job parameters:
	if 'af_platform' in i_params and len( i_params['af_platform']):
		job.setNeedOS( i_params['af_platform'])
	if 'af_priority' in i_params and i_params['af_priority'] >= 0:
		job.setPriority( i_params['af_priority'])
	if 'af_hostsmask' in i_params and len( i_params['af_hostsmask']):
		job.setHostsMask( i_params['af_hostsmask'])
	if 'af_hostsmask_exclude' in i_params and len( i_params['af_hostsmask_exclude']):
		job.setHostsMaskExclude( i_params['af_hostsmask_exclude'])
	if 'af_dependmask' in i_params and len( i_params['af_dependmask']):
		job.setDependMask( i_params['af_dependmask'])
	if 'af_dependmask_global' in i_params and len( i_params['af_dependmask_global']):
		job.setDependMaskGlobal( i_params['af_dependmask_global'])
	if 'af_job_paused' in i_params and i_params['af_job_paused']:
		job.setOffline()

	# Construct blocks travering through network:
	traverseChilds( job, i_params, prj)

	# Childs were reversed, see above
	job.blocks.reverse()

	return {'job':job,'prj':prj}


def traverseChilds( i_job, i_params, i_prj, i_mask = '', i_prefix = ''):

	mask = i_mask
	prefix = i_prefix

	for params in i_params['childs']:
		if params['afanasy']:
			prefix = params['nodelabel'] + '_'
			traverseChilds( i_job, params, i_prj, mask, prefix)
			if 'af_multi_independed' in i_params and not i_params['af_multi_independed']:
				mask = params['nodelabel'] + '.*'
		else:
			i_job.blocks.append( createBlock( i_params, params, i_prj, mask, i_prefix))
			if 'af_multi_independed' in i_params and not i_params['af_multi_independed']:
				mask = prefix + params['nodelabel']


def createBlock( i_afparams, i_wparams, i_prj, i_mask, i_prefix):

	block = af.Block( i_prefix + i_wparams['nodelabel'],'natron')
	block.setNumeric(
		i_afparams['af_frame_first'],
		i_afparams['af_frame_last'],
		i_afparams['af_frame_pertast'],
		i_afparams['af_frame_increment']
	)

	if 'af_frame_sequential' in i_afparams:
		block.setSequential( i_afparams['af_frame_sequential'])

	if 'af_capacity' in i_afparams and i_afparams['af_capacity'] != -1:
		block.setCapacity( i_afparams['af_capacity'])
	if 'af_maxtasks' in i_afparams and i_afparams['af_maxtasks'] != -1:
		block.setMaxRunningTasks( i_afparams['af_maxtasks'])
	if 'af_maxtasks_perhost' in i_afparams and i_afparams['af_maxtasks_perhost'] != -1:
		block.setMaxRunTasksPerHost( i_afparams['af_maxtasks_perhost'])

	if len( i_mask):
		if 'af_multi_wholerange' in i_afparams and i_afparams['af_multi_wholerange']:
			block.setDependMask( i_mask)
		else:
			block.setTasksDependMask( i_mask)

	block.setFiles( i_wparams['files'])

	cmd = os.getenv('NATRON_AF_RENDER','natron -b')
	cmd += ' -w "%s"' % i_wparams['nodename']
	cmd += ' @#@-@#@'
	cmd += ' "%s"' % i_prj

	block.setCommand( cmd)

	return block


def isNodeType( i_node, i_types):
	for tp in i_types:
		if i_node.getPluginID().split('.')[-1].lower().find( tp.lower()) == 0:
			return True
	return False

def isNodeAfanasy( i_node): return isNodeType( i_node,'afanasy')
def isNodeWrite(   i_node): return isNodeType( i_node,'write')

def getInputDot( i_node, i_index = 0):
	node = i_node.getInput( i_index)
	if node and isNodeType( node,['dot','noop']):
		return getInputDot( node)
	return node


def getInputs( i_node):
	o_nodes = [];
	i = 0
	while True:
		node = getInputDot( i_node, i)
		if not node: break
		if isNodeWrite( node) or isNodeAfanasy( node): o_nodes.append( node)
		i += 1
	return o_nodes


def onInputChanged( inputIndex, thisNode, thisGroup, app):

	print('"%s"[%d]' % ( thisNode.getLabel(), inputIndex ))


def renderSelected( i_app):
	'''
	Render selected node(s)
	'''

	# Collect selected Afanasy and Write nodes:
	sel_nodes = i_app.getSelectedNodes()
	nodes = []

	for node in sel_nodes:
		if isNodeWrite( node) or isNodeAfanasy( node): nodes.append( node)

	if len(nodes) == 0:
		NatronGui.natron.errorDialog('Error','No Afanasy or Write node(s) selected.')
		return

	# Get first and last frames:
	frame_first_min = None
	frame_first_max = None
	frame_last_min  = None
	frame_last_max  = None
	for node in nodes:
		if isNodeAfanasy( node):
			frame_first = node.getParam('af_frame_first').get()
			frame_last  = node.getParam('af_frame_last').get()
			if frame_first_min is None:
				frame_first_min = frame_first
				frame_first_max = frame_first
				frame_last_min  = frame_last
				frame_last_max  = frame_last
			else:
				if frame_first < frame_first_min: frame_first_min = frame_first
				if frame_first > frame_first_max: frame_first_max = frame_first
				if frame_last  < frame_last_min:  frame_last_min  = frame_last
				if frame_last  > frame_last_max:  frame_last_max  = frame_last
	# If no Afanasy selected get first and last frames from project settings:
	if frame_first_min is None:
		frame_first_min = i_app.timelineGetLeftBound()
		frame_first_max = i_app.timelineGetLeftBound()
		frame_last_min  = i_app.timelineGetRightBound()
		frame_last_max  = i_app.timelineGetRightBound()

	frame_first = str(frame_first_min)
	frame_last  = str(frame_last_min)
	if frame_first_min != frame_first_max:
		frame_first = '%d..%d' % (frame_first_min, frame_first_max)
	if frame_last_min != frame_last_max:
		frame_last  = '%d..%d' % (frame_last_min,  frame_last_max)

	# Create dialog:
	dialog = i_app.createModalDialog()
	fields = dict()
	fields['frame_first'     ] = dialog.createStringParam( 'frame_first','First Frame')
	fields['frame_last'      ] = dialog.createStringParam( 'frame_last','Last Frame')
	fields['af_frame_pertast'] = dialog.createIntParam(    'af_frame_pertast','Per Task')
	fields['af_job_paused'   ] = dialog.createBooleanParam('af_job_paused','Send Job Paused')

	fields['frame_first'  ].setDefaultValue( frame_first)
	fields['frame_last'   ].setDefaultValue( frame_last)
	fields['af_frame_pertast'].setDefaultValue( 1, 0)

	for field in fields:
		fields[field].setAnimationEnabled(False)

	dialog.refreshUserParamsGUI()

	if not dialog.exec_(): return

	# Get paramtets from dialog:
	params = dict()
	for field in fields:
		params[field] = fields[field].get()

	if params['frame_first'].find('..') == -1:
		params['af_frame_first'] = int(params['frame_first'])
	if params['frame_last'].find('..') == -1:
		params['af_frame_last'] = int(params['frame_last'])

	params['af_frame_increment'] = 1

	# Render nodes:
	renderNodes( i_app, nodes, params)


def getProjectPaths( i_app):

	data = i_app.getProjectParam('projectPaths').getValue()

	words = []
	for path in data.split('</Name>'):
		words.extend( path.split('</Value>'))

	names = []
	for word in words:
		word = word.replace('<Name>','')
		word = word.replace('<Value>','')
		word = word.strip()
		if len(word): names.append(word)

	paths = dict()
	i = 0
	while i < len(names):
		paths[names[i]] = names[i+1]
		i += 2

	return paths

def replaceProjectPaths( i_app, i_path):

	path = i_path
	paths = getProjectPaths( i_app)

	for name in paths:
		path = path.replace('[%s]' % name, paths[name])

	return path

