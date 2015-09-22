# Afanasy related tools

# For messages/warnings:
import NatronGui

# Just to reload Afanasy group before each creation:
import afanasyGroup

# Afanasy Python API:
import af


def createNode( app):
	reload( afanasyGroup)
	app.createNode('afanasy')


def onParamChanged( thisParam, thisNode, thisGroup, app, userEdited):

	#print( thisNode.getLabel(), thisParam.getScriptName())

	if thisParam.getScriptName() == 'afanasy_button_send_job':
		renderNodes( app, [thisNode])
	elif thisParam.getScriptName() == 'afanasy_button_frame_range':
		thisNode.getParam('af_frame_first').setValue( app.timelineGetLeftBound())
		thisNode.getParam('af_frame_last' ).setValue( app.timelineGetRightBound())


def renderNodes( i_app, i_nodes, i_params = None, i_store_frame_range = False):

	jobs_params = []

	for node in i_nodes:

		params = getAfParams( i_app, node, i_params)
		if params is None: return

		jobs_params.append( params)

	jobs = []

	for params in jobs_params:
		job = createJob( i_app, params)
		if job is None: return

		jobs.append( job)

	for job in jobs:
		job.output()

		if not job.send()[0]:
			NatronGui.natron.errorDialog('Error','Can`t send job to Afanasy server.\nSee script editor for details.')
			break


def getAfParams( i_app, i_node, i_params):

	o_params = dict()

	for par in i_node.getParams():
		name = par.getScriptName()
		if name.find('af_') != 0: continue
		o_params[name] = par.getValue()

	if len( o_params) == 0:
		NatronGui.natron.warningDialog('Error','No Afanasy parameters found on "%s"' % i_node.getLabel())
		return None;

	o_params['nodename'] = i_node.getLabel()
	o_params['filepath'] = i_app.getProjectParam('projectName').getValue()

	childs = getInputs( i_node)

	o_params['childs'] = []

	for child in childs:
		params = None
		if isNodeType( child,['write']):
			params = getWriteParams( child)
			if params is None: return None
			params['afanasy'] = False
		if isNodeType( child,['group']):
			params = getAfParams( i_app, child, i_params)
			if params is None: return None
			params['afanasy'] = True
		o_params['childs'].append( params)

	# Needed for depend masks, as first block should be most dependend
	o_params['childs'].reverse()

	return o_params


def getWriteParams( i_node):
	o_params = dict()

	pnames = ['filename']
	for pname in pnames:
		par = i_node.getParam(pname)
		if par:
			o_params[pname] = par.getValue()

	if len(o_params) == 0:
		NatronGui.natron.warningDialog('Error','No "filename" parameter found on "%s"' % i_node.getLabel())
		return None

	o_params['nodename'] = i_node.getLabel()

	return o_params


def createJob( i_app, i_params):

	name = i_params['af_job_name']
	if name == '':
		name = i_app.getProjectParam('projectName').getValue()

	job = af.Job( name)

	if len( i_params['af_platform']):
		job.setNeedOS( i_params['af_platform'])
	if i_params['af_priority'] >= 0:
		job.setPriority( i_params['af_priority'])
	if len( i_params['af_hostsmask']):
		job.setHostsMask( i_params['af_hostsmask'])
	if len( i_params['af_hostsmask_exclude']):
		job.setHostsMaskExclude( i_params['af_hostsmask_exclude'])
	if len( i_params['af_dependmask']):
		job.setDependMask( i_params['af_dependmask'])
	if len( i_params['af_dependmask_global']):
		job.setDependMaskGlobal( i_params['af_dependmask_global'])
	if i_params['af_job_paused']:
		job.setOffline()

	traverseChilds( job, i_params)

	# Childs were reversed, see above
	job.blocks.reverse()

	return job


def traverseChilds( i_job, i_params, i_mask = '', i_prefix = ''):

	mask = i_mask
	prefix = i_prefix

	for params in i_params['childs']:
		if params['afanasy']:
			prefix = params['nodename'] + '_'
			traverseChilds( i_job, params, mask, prefix)
			if not i_params['af_multi_independed']:
				mask = params['nodename'] + '.*'
		else:
			i_job.blocks.append( createBlock( i_params, params, mask, i_prefix))
			if not i_params['af_multi_independed']:
				mask = params['nodename']


def createBlock( i_afparams, i_wparams, i_mask, i_prefix):

	block = af.Block( i_prefix + i_wparams['nodename'],'natron')
	block.setNumeric(
		i_afparams['af_frame_first'],
		i_afparams['af_frame_last'],
		i_afparams['af_frame_pertast'],
		i_afparams['af_frame_increment']
	)
	block.setSequential( i_afparams['af_frame_sequential'])

	if len( i_mask):
		if i_afparams['af_multi_wholerange']:
			block.setDependMask( i_mask)
		else:
			block.setTasksDependMask( i_mask)

	cmd = 'natron'
	cmd += ' -w "%s"' % i_wparams['nodename']
	cmd += ' @#@-@#@'
	cmd += ' "%s"' % i_afparams['filepath']

	return block


def isNodeType( i_node, i_types):
	for tp in i_types:
		if i_node.getPluginID().split('.')[-1].lower().find( tp.lower()) == 0:
			return True
	return False


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
		if isNodeType( node,['write','group']): o_nodes.append( node)
		i += 1
	return o_nodes


def onInputChanged( inputIndex, thisNode, thisGroup, app):

	print('"%s"[%d]' % ( thisNode.getLabel(), inputIndex ))

