#!/usr/bin/env python

import os, shutil, signal, sys, time

import af

from optparse import OptionParser

Extensions = ['jpg','png','dpx']
TmpFiles = 'img.%07d.jpg'

Parser = OptionParser(usage="%prog [options] input\n\
   Pattern examples = \"img.####.jpg\" or \"img.%04d.jpg\".\n\
   Type \"%prog -h\" for help", version="%prog 1.0")

Parser.add_option('-i', '--inputs',     dest='inputs',     type  ='string', default='RESULT/JPG',help='Inputs')
Parser.add_option('-d', '--dest',       dest='dest',       type  ='string', default='',          help='Destination')
Parser.add_option('-u', '--afuser',     dest='afuser',     type  ='string', default='',          help='Afanasy user name')
Parser.add_option('-t', '--testonly',   dest='testonly',   action='store_true', default=False,   help='Test input only')
Parser.add_option('-V', '--verbose',    dest='verbose',    action='store_true', default=False,   help='Verbose mode')

print('{"put":[')

def errExit( i_msg):
	print('{"error":"%s"},' % i_msg)
	print('{"status":"error"}]}')
	sys.exit(1)

def interrupt( signum, frame):
	errExit('Interrupt received')
signal.signal(signal.SIGTERM, interrupt)
signal.signal(signal.SIGABRT, interrupt)
signal.signal(signal.SIGINT,  interrupt)

(Options, args) = Parser.parse_args()

if len(args) < 1:
	errExit('Not enough arguments provided.')

if Options.dest == '': errExit('Destination is not specified')

Inputs = Options.inputs.split(',')
Sources = args

if not Options.testonly:
	if not os.path.isdir( Options.dest ):
		errExit('Destination folder does not exist:\n' + Options.dest )

commands = []
task_names = []
CmdPut = os.path.join( os.path.dirname( sys.argv[0]), 'put.py')
CmdPut = 'python "%s"' % os.path.normpath( CmdPut)
CmdPut += ' -d "%s"' % Options.dest

for src in Sources:

	folders = []
	for folder in Inputs:
		folder = os.path.join( src, folder)
		if not os.path.isdir( folder): continue

		for item in os.listdir( folder):
			if item[0] in '._': continue
			item = os.path.join( folder, item)
			if os.path.isdir( item):
					folders.append( item)

	if len(folders):
		folders.sort()
		folder = folders[-1]
	else:
		errExit('Input not founded for: %s' % src)

	name = os.path.basename( src)
	version = os.path.basename( folder).replace( name, '')
	name += '_' + version
	dest = os.path.join( Options.dest, name)

	print('{"src":"%s","name":"%s","version":"%s","dst":"%s"},' % (folder, name, version, dest))

	cmd = CmdPut
	cmd += ' -s "%s"' % folder
	cmd += ' -d "%s"' % Options.dest
	cmd += ' -n "%s"' % name

	commands.append( cmd)
	task_names.append( name)

	if Options.verbose: print( cmd)

print('{"progress":"%d sequences founded"},' % len(Sources))

job = af.Job('PUT ' + Options.dest)
block = af.Block('put')
counter = 0
for cmd in commands:
	task = af.Task( task_names[counter])
	task.setCommand( cmd)
	block.tasks.append( task)
	counter += 1
block.setCapacity(100)
block.setMaxRunTasksPerHost(1)
job.blocks.append( block)

if not Options.testonly:
	if not job.send():
		errExit('Can`t send job to server.')

print('{"status":"success"}]}')
