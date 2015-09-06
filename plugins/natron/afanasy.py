# Afanasy related tools

import af

import afanasyGroup


def createNode( app):
	reload( afanasyGroup)
	app.createNode('afanasy')


def onParamChanged( thisParam, thisNode, thisGroup, app, userEdited):

	print( thisParam, thisNode, thisGroup, app, userEdited)

	# Button param is none !?
	if thisParam is not None: return

	renderNodes( app, [thisNode])


def renderNodes( i_app, i_nodes, i_params = None, i_store_frame_range = False):

	jobs_params = []

	for node in i_nodes:

		params = dict()
		getAfParams( node, params)
		print( params)

		afanas, writes = getInputs( node)

		#print('Writes:');    for node in writes: print( node.getLabel() + ': ' + node.getScriptName() + ': ' + node.getPluginID())
		#print('Afanasies:'); for node in afanas: print( node.getLabel() + ': ' + node.getScriptName() + ': ' + node.getPluginID())

		jobs_params.append( params)

	jobs = []

	for params in jobs_params:
		job = createJob( i_app, params)
		if job is None:
			return
		jobs.append( job)

	for job in jobs:

		job.output()

		#job.send()


def createJob( i_app, i_params):

	name = i_params['af_job_name']
	if name == '':
		name = i_app.getProjectParam('projectName').getValue()

	job = af.Job( name)

	return job


def getAfParams( i_node, o_params):
	for par in i_node.getParams():
		name = par.getScriptName()
		if name.find('af_') != 0: continue
		o_params[name] = par.getValue()


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
	o_afanas = [];
	o_writes = [];
	i = 0
	while True:
		node = getInputDot( i_node, i)
		if not node: break
		if isNodeType( node,['write']  ): o_writes.append( node)
		if isNodeType( node,['afanasy']): o_afanas.append( node)
		i += 1
	return o_afanas, o_writes


def onInputChanged( inputIndex, thisNode, thisGroup, app):

	print( inputIndex, thisNode, thisGroup, app)

