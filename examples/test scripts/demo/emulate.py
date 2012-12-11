#!/bin/env python

import json, os, random, sys, shutil, time

import af, afnetwork

JobNamePrefix = '_emulate_'
JobUserPrefix = '_emuser_'
TmpDir = '/tmp/afanasy_emulate'
OutputDir = '/var/tmp/afanasy'
TasksOutDir = JobNamePrefix+'/tasksoutput/'+JobNamePrefix

ActionUser = 'jimmy'
ActionHost = 'pc01'
JobActions = ['stop','restart','delete']

from optparse import OptionParser
Parser = OptionParser(usage="usage: %prog [options]", version="%prog 1.0")
Parser.add_option('-j', '--jobspack',  dest='jobspack',  type='int',   default=10,   help='jobs numbder in pack')
Parser.add_option('-b', '--blocksnum', dest='blocksnum', type='int',   default=2,    help='blocks numbder in each job')
Parser.add_option('-n', '--tasksnum',  dest='tasksnum',  type='int',   default=10,   help='tasks numbder in each job block')
Parser.add_option('-u', '--usersnum',  dest='usersnum',  type='int',   default=10,   help='users number')
Parser.add_option('-t', '--cycletime', dest='cycletime', type='float', default=5.0,  help='cycle pause time in seconds')
Parser.add_option('-p', '--periods',   dest='periods',   type='int',   default=10,   help='number of cycles in period')
Parser.add_option('-d', '--delperiod', dest='delperiod', type='int',   default=3,    help='deletion period')
Parser.add_option(      '--pausetime', dest='pausetime', type='float', default=15.0, help='pause time in seconds')
Parser.add_option(      '--notasks',   dest='notasks',   action='store_true', default=False, help='empty tasks')
Parser.add_option(      '--nopost',    dest='nopost',    action='store_true', default=False, help='no post command')
Parser.add_option('-c', '--cleanup',   dest='cleanup',   action='store_true', default=False, help='cleanup mode')
Options, Args = Parser.parse_args()

def sendAction( i_type, i_mask, i_operation, i_params = None):
	action = dict()
	action['type'] = i_type
	action['user_name'] = ActionUser
	action['host_name'] = ActionHost
	action['mask'] = i_mask

	if i_operation is not None:
		operation = dict()
		operation['type'] = i_operation
		action['operation'] = operation

	if i_params is not None:
		action['params'] = i_params

	afnetwork.sendServer( json.dumps( {'action': action}), False, False)

def jobsDelete():
	sendAction('jobs', JobNamePrefix+'.*', 'delete')

def jobRandomAction( jobs):	
	job = jobs[ int( len(jobs) * random.random())]
	action = None
	operation = None
	if len(jobs) % 2 == 0:
		action = JobActions[ int( len(JobActions) * random.random())]
	else:
		operation = {"priority": int(200*random.random())}
	sendAction('jobs', job, action, operation)

# Temporary folder:
if os.path.isdir( TmpDir):
	shutil.rmtree( TmpDir)

# Cleanup previous tasks outputs:
if os.path.isdir( OutputDir):
	os.system('rm -rf %s.*' % TasksOutDir )

# Cleanup mode:
if Options.cleanup:
	print('Clean up completed.')
	sys.exit(0)

print('Jobs Pack	= %d' % Options.jobspack )
print('Tasks Number = %d' % Options.tasksnum )
if Options.nopost: print('No post command')

# Create temporary folder:
os.mkdir( TmpDir)

# Create a job template:
job = af.Job()
for b in range( 0, Options.blocksnum):
	block = af.Block()
	job.blocks.append( block)
	if not Options.notasks:
		block.setNumeric(1, Options.tasksnum)
		block.setNonSequential()
		cmd = 'task.py'
		cmd = os.path.join( os.getcwd(), cmd)
		cmd = 'python "%s"' % cmd
		cmd += ' -s @#@ -e @#@ -t 1 @####@ @#####@ @#####@ @#####@'
		block.setCommand( cmd, False)
		block.setFiles('file_a.@#@.@####@;file_b.@#@.@####@')


job.setNeedOS('')

counter = 0
user_num = 0
period = 0
deletion = 0
jobs = []
while( 1):
	for j in range( 0, Options.jobspack):
		jobname = '%(JobNamePrefix)s.%(period)d.%(counter)d' % vars()
		username = '%(JobUserPrefix)s%(user_num)d' % vars()
		output = 'Job = %(counter)d, user = %(username)s, period = %(period)d' % vars()
		print( output)
		tmpfile = TmpDir+'/'+jobname

		if Options.nopost:
			job.data['command_post'] = '';
		else:
			file = open( tmpfile, 'w')
			file.write( output)
			file.close()
			job.data['command_post'] = 'rm "%s"' % tmpfile;

		job.data['name'] = jobname
		job.data['user_name'] = username
		job.data['time_creation'] = int(time.time())

		if not job.send():
			print('Error creating a job, exiting.')
			sys.exit(1)

		jobs.append( jobname)

#		if counter % 11 == 0: jobRandomAction( jobs);
		jobRandomAction( jobs);

		user_num += 1
		if user_num >= Options.usersnum:
			user_num = 0

		counter += 1

	time.sleep( Options.cycletime )

	period += 1
	if period >= Options.periods:
		period = 0
		deletion += 1
		if deletion >= Options.delperiod:
			deletion = 0
			print('Deleting Jobs.')
			for d in range(0,10):
				jobsDelete()
				time.sleep(1.0)
			jobs = []
		else:
			print('Pause...')
			time.sleep( Options.pausetime)

