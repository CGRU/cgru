#!/usr/bin/env python

import os, re, shutil, signal, sys, time

import af

from optparse import OptionParser

Extensions = ['jpg','png','dpx']
TmpFiles = 'img.%07d.jpg'

Parser = OptionParser(usage="%prog [options] input\n\
   Pattern examples = \"img.####.jpg\" or \"img.%04d.jpg\".\n\
   Type \"%prog -h\" for help", version="%prog 1.0")

Parser.add_option('-p', '--padding',      dest='padding',      type  ='int',    default=3,           help='Version padding')
Parser.add_option('-i', '--inputs',       dest='inputs',       type  ='string', default='RESULT/JPG',help='Inputs')
Parser.add_option('-d', '--dest',         dest='dest',         type  ='string', default='',          help='Destination')
Parser.add_option('-u', '--afuser',       dest='afuser',       type  ='string', default='put',       help='Afanasy user name')
Parser.add_option('-m', '--afmaxtasks',   dest='afmaxtasks',   type  ='int',    default=5,           help='Afanasy max tasks')
Parser.add_option('-c', '--afcapacity',   dest='afcapacity',   type  ='int',    default=0,           help='Afanasy capacity')
Parser.add_option('-s', '--skipexisting', dest='skipexisting', action='store_true', default=False,   help='Skip existing folders')
Parser.add_option('-e', '--skiperrors',   dest='skiperrors',   action='store_true', default=False,   help='Skip error folders')
Parser.add_option('-t', '--testonly',     dest='testonly',     action='store_true', default=False,   help='Test input only')
Parser.add_option('-V', '--verbose',      dest='verbose',      action='store_true', default=False,   help='Verbose mode')

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

SimilarCharactrers = ' .-()[]{}!'
def simiralName( i_name):
	name = i_name.lower()
	for c in SimilarCharactrers:
		name = name.replace( c,'_')
	return name

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
CmdPut = os.environ['CGRU_LOCATION'] + '/utilities/put.py'
CmdPut = 'python "%s"' % os.path.normpath( CmdPut)
CmdPut += ' -d "%s"' % Options.dest

for src in Sources:

	folder = None
	version = None
	name = os.path.basename( src)

	for inp in Inputs:
		inp = os.path.join( src, inp)
		if not os.path.isdir( inp): continue

		for item in os.listdir( inp):
			if item[0] in '._': continue
			if not os.path.isdir( os.path.join( inp, item)): continue
			ver = simiralName( item).replace( simiralName( name), '').strip('!_-. ')
			ver_digits = re.findall(r'\d+', ver)
			if len( ver_digits ):
				ver_digits = ver_digits[0]
				ver_number = int( ver_digits)
				ver_number = ('%0'+str(Options.padding)+'d') % ver_number
				ver = ver.replace( ver_digits, ver_number, 1)

			if len( ver) and ver[0].isdigit(): ver = 'v' + ver

			if version is not None:
				if version >= ver: continue
			version = ver

			folder = os.path.join( inp, item)

	if folder is None:
		if Options.skiperrors:
			print('{"error":"%s"},' % src)
			continue
		else:
			errExit('Input not founded for: %s' % src)

	if version == '': version = ('v%0'+str(Options.padding)+'d') % 0
	name += '_' + version

	dest = os.path.join( Options.dest, name)
	skipexisting = False
	skipexisting_str = 'false'
	if os.path.isdir( dest ) and Options.skipexisting:
		skipexisting = True
		skipexisting_str = 'true'

	print('{"src":"%s","name":"%s","version":"%s","dst":"%s","skipexisting":%s},' % (folder, name, version, dest, skipexisting_str))

	if skipexisting: continue

	cmd = CmdPut
	cmd += ' -s "%s"' % folder
	cmd += ' -d "%s"' % Options.dest
	cmd += ' -n "%s"' % name

	commands.append( cmd)
	task_names.append( name)

	if Options.verbose: print( cmd)

print('{"progress":"%d sequences founded"},' % len(commands))

job = af.Job('PUT ' + Options.dest)
job.setUserName( Options.afuser)
job.setMaxRunningTasks( Options.afmaxtasks)
job.setMaxRunTasksPerHost(1)

block = af.Block('put')
counter = 0
for cmd in commands:
	task = af.Task( task_names[counter])
	task.setCommand( cmd)
	block.tasks.append( task)
	counter += 1
if Options.afcapacity > 0: block.setCapacity( Options.afcapacity)
job.blocks.append( block)

if not Options.testonly:
	if not job.send():
		errExit('Can`t send job to server.')

print('{"status":"success"}]}')
