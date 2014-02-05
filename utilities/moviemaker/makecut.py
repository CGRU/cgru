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
Parser.add_option('-n', '--cutname',    dest='cutname',    type  ='string', default='',          help='Cut name')
Parser.add_option('-f', '--fps',        dest='fps',        type  ='string', default='24',        help='FPS')
Parser.add_option('-r', '--resolution', dest='resolution', type  ='string', default='1280x720',  help='Resolution: 1280x720')
Parser.add_option('-c', '--codec',      dest='codec',      type  ='string', default='h264_good', help='Codec')
Parser.add_option('-o', '--outdir',     dest='outdir',     type  ='string', default='cut',       help='Output folder')
Parser.add_option('-u', '--afuser',     dest='afuser',     type  ='string', default='',          help='Afanasy user name')
Parser.add_option('-t', '--testonly',   dest='testonly',   action='store_true', default=False,   help='Test input only')

def errExit( i_msg):
	print('{"error":"%s"},' % i_msg)
	print('{"status":"error"}]}')
	sys.exit(1)

def interrupt( signum, frame):
	errExit('Interrupt received')
signal.signal(signal.SIGTERM, interrupt)
signal.signal(signal.SIGABRT, interrupt)
signal.signal(signal.SIGINT,  interrupt)

print('{"cut":[')

(Options, args) = Parser.parse_args()

if len(args) < 1:
	errExit('Not enough arguments provided.')

Inputs = Options.inputs.split(',')
Shots = args
CutName = Options.cutname

if os.path.isfile( args[-1]):
	Shots = args[-1]
	if CutName == '':
		CutName = os.path.basename( Shots)
	file = open( Shots)
	Shots = file.readlines()
	file.close

if len( Shots) < 2:
	errExit('Less than 2 shots provided.')

if CutName == '':
	CutName = os.path.basename( os.path.dirname( Shots[0]))

ftime = time.time()
OutDir = Options.outdir + '/' + CutName
OutDir = os.path.normpath( OutDir)
if not Options.testonly:
	if os.path.isdir( OutDir):
		print('{"progress":"Deleting folder: %s"},' % OutDir )
		shutil.rmtree( OutDir)
	os.makedirs( OutDir)
	print('{"progress":"Creating folder: %s"},' % OutDir )

movie_name = os.path.basename(CutName) + time.strftime('_%y-%m-%d_%H-%M-%S')
movie_name = os.path.join( Options.outdir, movie_name)

commands = []
task_names = []
#cmd_prefix = os.environ['CGRU_LOCATION'] + '/utilities/moviemaker/makeframe.py'
cmd_prefix = os.path.join( os.path.dirname( sys.argv[0]), 'makeframe.py')
cmd_prefix = 'python "%s"' % os.path.normpath( cmd_prefix)
cmd_prefix += ' -t "dailies"'
cmd_prefix += ' -r %s' % Options.resolution
cmd_prefix += ' -d "%s"' % time.strftime('%y-%m-%d')

file_counter = 0

for shot in Shots:
	shot = shot.strip()
	if len(shot) == 0: continue
	if shot[0] == '#': continue

	for folder in Inputs:
		folder = os.path.join( shot, folder)
		if os.path.isdir( folder): break
		else: folder = None
	
	if folder is None:
		errExit('Input not founded for: %s' % shot)

	items = []
	for item in os.listdir( folder):
		if os.path.isdir( os.path.join( folder, item)):
			if item[0] == 'v':
				items.append( item)
	if len(items):
		items.sort()
		folder = os.path.join( folder, items[-1])
	print('{"sequence":"%s"},' % folder)

	files = []
	for item in os.listdir( folder):
		valid = False
		for ext in Extensions:
			if item[-len(ext):].lower() == ext:
				valid = True
				break
		if valid:
			files.append( os.path.join( folder, item))
	if len(files) == 0:
		errExit('No files founded in folder: %s' % folder)
	files.sort()

	for image in files:
		cmd = cmd_prefix
		cmd += ' --project "%s"' % CutName
		cmd += ' --shot "%s"' % os.path.basename( shot)
		cmd += ' --ver "%s"' % os.path.basename( folder)
		cmd += ' --moviename "%s"' % os.path.basename( movie_name)
		cmd += ' -f "%s"' % os.path.basename( image)
		cmd += ' "%s"' % image
		output = os.path.join( OutDir, TmpFiles % file_counter)
		cmd += ' "%s"' % output

		file_counter += 1
		commands.append( cmd)
		task_names.append( os.path.basename(image))

cmd_encode = os.path.join( os.path.dirname( sys.argv[0]), 'makemovie.py')
cmd_encode = 'python "%s"' % os.path.normpath( cmd_encode)
cmd_encode += ' -f %s' % Options.fps
cmd_encode += ' -c %s' % Options.codec
cmd_encode += ' "%s"' % os.path.join( OutDir, TmpFiles)
cmd_encode += ' "%s"' % movie_name

job = af.Job('CUT ' + CutName)
block = af.Block('convert')
counter = 0
for cmd in commands:
	task = af.Task( task_names[counter])
	task.setCommand( cmd)
	block.tasks.append( task)
	counter += 1
block.setCapacity(100)
block.setMaxRunTasksPerHost(2)
block.setTasksMaxRunTime(20)
job.blocks.append( block)

block = af.Block('encode')
block.setDependMask('convert')
task = af.Task('encode')
task.setCommand( cmd_encode)
block.tasks.append( task)
job.blocks.append( block)

if Options.afuser != '': job.setUserName( Options.afuser)

job.setNeedOS('win')
if not Options.testonly:
	if not job.send():
		errExit('Can`t send job to server.')

print('{"status":"success"}]}')
