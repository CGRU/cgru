#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import sys

import af

from optparse import OptionParser

ImgTypes = ['jpg', 'jpeg', 'dpx', 'cin', 'exr', 'tif', 'tiff', 'tga', 'png']

Parser = OptionParser(
	usage="%prog [options] input\ntype \"%prog -h\" for help",
	version="%prog 1.0"
)

Parser.add_option('-t', '--type',       dest='type',       type  ='string',     default='jpg',  help='Image type')
Parser.add_option('-c', '--colorspace', dest='colorspace', type  ='string',     default='auto', help='Input images colorspace')
Parser.add_option('-r', '--resize',     dest='resize',     type  ='string',     default='',     help='Resize (1280x720)')
Parser.add_option('-q', '--quality',    dest='quality',    type  ='int',        default=75,     help='Quality')
Parser.add_option('-o', '--output',     dest='output',     type  ='string',     default='',     help='Output image')
Parser.add_option('-A', '--afanasy',    dest='afanasy',    action='store_true', default=False,  help='Use Afanasy')
Parser.add_option(      '--afuser',     dest='afuser',     type  ='string',     default='',     help='Afanasy user')
Parser.add_option(      '--afmax',      dest='afmax',      type  ='int',        default='-1',   help='Afanasy maximum running tasks')
Parser.add_option(      '--afmph',      dest='afmph',      type  ='int',        default='-1',   help='Afanasy max tasks per host')
Parser.add_option(      '--afmrt',      dest='afmrt',      type  ='int',        default='-1',   help='Afanasy max running time')
Parser.add_option(      '--afcap',      dest='afcap',      type  ='int',        default='-1',   help='Afanasy capacity')
Parser.add_option('-I', '--identify',   dest='identify',   action='store_true', default=False,  help='Identify image')
Parser.add_option('-J', '--json',       dest='json',       action='store_true', default=False,  help='Output JSON summary')
Parser.add_option('-V', '--verbose',    dest='verbose',    action='store_true', default=False,  help='Verbose mode')
Parser.add_option('-D', '--debug',      dest='debug',      action='store_true', default=False,  help='Debug mode')

Options, Args = Parser.parse_args()

OUT = dict()
OUT['convert'] = []


def errorExit(i_err):
	OUT['error'] = i_err
	print(json.dumps(OUT))
	sys.exit(1)


if len(Args) < 1:
	errorExit('Input is not specified.')
Inputs = Args

Jobs = []
MkDirs = []
FilesIn = []
JobNames = []

for input in Inputs:
	files = []
	if os.path.isfile(input):
		files = [input]
	elif os.path.isdir(input):
		for afile in os.listdir(input):
			if afile[0] == '.':
				continue
			afile = os.path.join(input, afile)
			if not os.path.isfile(afile):
				continue
			files.append(afile)
		files.sort()
	else:
		errorExit('%s not found.' % input)

	cmds = []
	mkdir = None
	files_in = []

	output = Options.output
	if output == '' or len(Inputs) > 1:
		output = input
		if Options.type == 'jpg':
			output += '.q%d' % Options.quality
		if Options.resize != '':
			output += '.r%s' % Options.resize
#		output += '.' + Options.type

	if os.path.isdir(input):
		mkdir = '%s.%s' % (output, Options.type )

	for afile in files:
		imgtype = afile.rfind('.')
		if imgtype == 1:
			continue
		imgtype = afile[imgtype + 1:].lower()
		if not imgtype in ImgTypes:
			continue

		cmd = 'convert'

		if Options.afanasy:
			cmd += ' -identify -verbose'
		elif Options.identify:
			cmd += ' -identify'

		cmd += ' "%s"' % afile
		cmd += ' -flatten'

		if Options.colorspace != 'auto':
			if Options.colorspace == 'extension':
				if imgtype == 'exr':
					cmd += ' -set colorspace RGB'
				elif imgtype == 'dpx':
					cmd += ' -set colorspace Log'
				elif imgtype == 'cin':
					cmd += ' -set colorspace Log'
				else:
					cmd += ' -set colorspace sRGB'
			else:
				cmd += ' -set colorspace ' + Options.colorspace

		if Options.resize != '':
			cmd += ' -resize %s' % Options.resize

		ext = Options.type
		colorspace = ' -colorspace sRGB'
		if Options.type == 'jpg':
			cmd += ' -quality %d' % Options.quality
			cmd += ' -colorspace sRGB'
		elif Options.type == 'dpx':
			cmd += ' -depth 10'
			cmd += ' -colorspace Log'
		elif Options.type == 'tif8':
			ext = 'tif'
			cmd += ' -depth 8'
			cmd += ' -colorspace sRGB'
		elif Options.type == 'tif16':
			ext = 'tif'
			cmd += ' -depth 16'
			cmd += ' -colorspace sRGB'
		elif Options.type == 'exr':
			cmd += ' -colorspace RGB'

		cmd += colorspace

		if mkdir:
			output = os.path.join(mkdir, os.path.basename(afile)) + '.' + ext

		cmd += ' "%s"' % output

		cmds.append(cmd)
		files_in.append(afile)

	convert = dict()
	convert['input'] = input
	convert['files_num'] = len(files_in)
	if mkdir:
		convert['output'] = mkdir
		convert['type'] = 'folder'
	else:
		convert['output'] = output
		convert['type'] = 'file'

	if len(cmds):
		Jobs.append(cmds)
		MkDirs.append(mkdir)
		FilesIn.append(files_in)
		JobNames.append(os.path.basename(convert['output']))
	else:
		convert['warning'] = 'No images found'

	OUT['convert'].append(convert)

for i in range(0, len(Jobs)):

	if MkDirs[i]:
		if Options.verbose:
			print('mkdir ' + MkDirs[i])
		if not Options.debug and not os.path.isdir(MkDirs[i]):
			os.makedirs(MkDirs[i])

	if Options.afanasy:
		job = af.Job('CVT ' + JobNames[i])
		block = af.Block('convert')
		job.blocks.append(block)

		if Options.afuser != '':
			job.setUserName(Options.afuser)

		if Options.afcap != -1:
			block.setCapacity(Options.afcap)

		if Options.afmax != -1:
			block.setMaxRunningTasks(Options.afmax)

		if Options.afmph != -1:
			block.setMaxRunTasksPerHost(Options.afmph)

		if Options.afmrt != -1:
			block.setTasksMaxRunTime(Options.afmrt)

	for j in range(0, len(Jobs[i])):
		if Options.verbose or Options.debug:
			print(Jobs[i][j])

		if Options.afanasy:
			task = af.Task(os.path.basename(FilesIn[i][j]))
			block.tasks.append(task)
			task.setCommand(Jobs[i][j])

		if not Options.afanasy and not Options.debug:
			os.system(Jobs[i][j])

	if Options.afanasy:
		if Options.verbose or Options.debug:
			job.output()
		if not Options.debug:
			job.send()

if Options.json:
	print(json.dumps(OUT))
