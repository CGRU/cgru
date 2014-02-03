#!/usr/bin/env python

import os, shutil, signal, sys, time

import af

from optparse import OptionParser

Extensions = ['jpg','png']
Folder = 'RESULT/JPG'
TmpFiles = 'img.%07d.jpg'

Parser = OptionParser(usage="%prog [options] input\n\
   Pattern examples = \"img.####.jpg\" or \"img.%04d.jpg\".\n\
   Type \"%prog -h\" for help", version="%prog 1.0")

Parser.add_option('-n', '--cutname',    dest='cutname',    type  ='string', default='',         help='Cut name')
Parser.add_option('-r', '--resolution', dest='resolution', type  ='string', default='1280x720', help='Resolution: 1280x720')
Parser.add_option('-o', '--outdir',     dest='outdir',     type  ='string', default='cut',      help='Output folder')
Parser.add_option('-u', '--afuser',     dest='afuser',     type  ='string', default='',         help='Afanasy user name')

def interrupt( signum, frame):
   print('\nInterrupt received...')
   exit(0)
signal.signal(signal.SIGTERM, interrupt)
signal.signal(signal.SIGABRT, interrupt)
signal.signal(signal.SIGINT,  interrupt)

(Options, args) = Parser.parse_args()

if len(args) < 1: Parser.error('Not enough arguments provided.')

Scenes = args
CutName = Options.cutname

if os.path.isfile( args[-1]):
	Scenes = args[-1]
	if CutName == '':
		CutName = os.path.basename( Scenes)
	print('Reading scenes file "%s"...' % Scenes)
	file = open( Scenes)
	Scenes = file.readlines()
	file.close

if len( Scenes) < 2:
	print('Error: Less than 2 scenes provided.')
	exit(1)

if CutName == '':
	CutName = os.path.basename( os.path.dirname( Scenes[0]))

ftime = time.time()
OutDir = Options.outdir + '/' + CutName
OutDir = os.path.normpath( OutDir)
if os.path.isdir( OutDir):
	print('Deleting folder "%s"...' % OutDir)
	shutil.rmtree( OutDir)
os.makedirs( OutDir)

movie_name = os.path.basename(CutName) + time.strftime('_%y-%m-%d_%H-%M-%S.mov')
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

for scene in Scenes:
	scene = scene.strip()
	if len(scene) == 0: continue
	if scene[0] == '#': continue
	folder = scene
	folderDeeper = os.path.join( folder, Folder)
	if os.path.isdir( folderDeeper):
		folder = folderDeeper
	if not os.path.isdir( folder):
		print('Error: Folder not founded:\n%s' % folder)
		exit(1)

	items = []
	for item in os.listdir( folder):
		if os.path.isdir( os.path.join( folder, item)):
			if item[0] == 'v':
				items.append( item)
	if len(items):
		items.sort()
		folder = os.path.join( folder, items[-1])
	print( folder)

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
		print('Error: No files founded in folder:\n"%s"' % folder)
		exit(1)
	files.sort()

	for image in files:
		cmd = cmd_prefix
		cmd += ' --project "%s"' % CutName
		cmd += ' --shot "%s"' % os.path.basename( scene)
		cmd += ' --ver "%s"' % os.path.basename( folder)
		cmd += ' --moviename "%s"' % os.path.basename( movie_name)
		cmd += ' -f "%s"' % os.path.basename( image)
		cmd += ' "%s"' % image
		output = os.path.join( OutDir, TmpFiles % file_counter)
		cmd += ' "%s"' % output

		file_counter += 1
		commands.append( cmd)
		task_names.append( os.path.basename(image))

cmd_encode = 'ffmpeg -y -r 24 -i "' + os.path.join( OutDir, TmpFiles) + '" -vcodec mjpeg -pix_fmt yuv420p -qscale 1 -r 24 "' + movie_name + '"'

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
task = af.Task('ffmpeg')
task.setCommand( cmd_encode)
block.tasks.append( task)
job.blocks.append( block)

if Options.afuser != '': job.setUserName( Options.afuser)

job.setNeedOS('win')
job.send()

#if os.path.isdir( OutDir): shutil.rmtree( OutDir)
