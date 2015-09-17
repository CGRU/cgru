# Afanasy related tools

import af

import afanasyGroup


def createNode( app):
	reload( afanasyGroup)
	app.createNode('afanasy')


def onParamChanged( thisParam, thisNode, thisGroup, app, userEdited):

	print( thisParam, thisNode, thisGroup, app, userEdited)

	if thisParam.getScriptName() == 'afanasy_button_send_job':
		renderNodes( app, [thisNode])


def renderNodes( i_app, i_nodes, i_params = None, i_store_frame_range = False):

	jobs_params = []

	for node in i_nodes:

		params = getAfParams( i_app, node, i_params)
		print( params)
		if params is None: return

		jobs_params.append( params)

	jobs = []

	for params in jobs_params:
		job = createJob( i_app, params)
		if job is None: return

		jobs.append( job)

	for job in jobs:

		job.output()

		#job.send()


def getAfParams( i_app, i_node, i_params):
	o_params = dict()
	o_params['nodename'] = i_node.getLabel()
	o_params['filepath'] = i_app.getProjectParam('projectName').getValue()

	for par in i_node.getParams():
		name = par.getScriptName()
		if name.find('af_') != 0: continue
		o_params[name] = par.getValue()

	childs = getInputs( i_node)

	o_params['childs'] = []

	for child in childs:
		params = None
		if isNodeType( child,['write']):
			params = getWriteParams( child)
			if params is None: return None
			params['afanasy'] = False
		if isNodeType( child,['afanasy']):
			params = getAfParams( child, i_params)
			if params is None: return None
			params['afanasy'] = True
		o_params['childs'].append( params)

	return o_params


def getWriteParams( i_node):
	o_params = dict()
	o_params['nodename'] = i_node.getLabel()

	pnames = ['filename']
	for pname in pnames:
		par = i_node.getParam(pname)
		if par:
			o_params[pname] = par.getValue()

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

	for child in i_params['childs']:
		if child['afanasy']:
			block = createBlock( i_app, i_params, child)
		else:
			block = createBlock( i_app, i_params, child)

	job.blocks.append( block)

	return job


def createBlock( i_app, i_afparams, i_wparams):

	block = af.Block( i_wparams['nodename'],'natron')
	block.setNumeric(
		i_afparams['af_frame_first'],
		i_afparams['af_frame_last'],
		i_afparams['af_frame_pertast'],
		i_afparams['af_frame_increment']
	)
	block.setSequential('af_frame_sequential')

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
		if isNodeType( node,['write','afanasy']): o_nodes.append( node)
		i += 1
	return o_nodes


def onInputChanged( inputIndex, thisNode, thisGroup, app):

	print( inputIndex, thisNode, thisGroup, app)

