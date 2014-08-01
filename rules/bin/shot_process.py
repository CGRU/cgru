#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import re
import os
import signal
import sys

from optparse import OptionParser

ImgTypes = ['jpg','png','exr','dpx','tif','tiff','cin','tga']
#MovTypes = ['mov','avi','mpg','mpeg','mp4']

Parser = OptionParser(
	usage="%prog [options]\n\tType \"%prog -h\" for help",
	version="%prog 1.0"
)

Parser.add_option('-f', '--fps',      dest='fps',      type='int',          default=24,     help='Frames per second')
Parser.add_option('-o', '--output',   dest='output',   type='string',       default='WORK', help='Software output')
Parser.add_option('-s', '--soft',     dest='soft',     type='string',       default='',     help='Software type')
Parser.add_option('-t', '--template', dest='template', type='string',       default='',     help='Software file template')
Parser.add_option('-r', '--run',      dest='run',      type='string',       default='',     help='Run soft')
Parser.add_option('-V', '--verbose',  dest='verbose',  action='store_true', default=False,  help='Verbose mode')
Parser.add_option('-D', '--debug',    dest='debug',    action='store_true', default=False,  help='Debug mode')

Out = []

def errExit(i_msg):
	Out.append({'error': i_msg})
	Out.append({'status': 'error'})
	print(json.dumps({'shot_process': Out}, indent=4))
	sys.exit(1)


def interrupt(signum, frame):
	errExit('Interrupt received')

signal.signal(signal.SIGTERM, interrupt)
signal.signal(signal.SIGABRT, interrupt)
signal.signal(signal.SIGINT, interrupt)

(Options, Args) = Parser.parse_args()
if Options.debug: Options.verbose = True

if len( Args) == 0:
	errExit('Paths are not specified.')

TemplateFileData = ''
if Options.template != '':
	if not os.path.isfile( Options.template):
		errExit('Template file does not exist')
	file = open( Options.template,'r')
	TemplateFileData = file.read()
	file.close()

def isImage( i_file):
	name,ext = os.path.splitext( i_file)
	if ext[1:].lower() in ImgTypes:
		return True
	return False

def scanSequences( i_files):
	"""search an array of filenames for sequences

	:param i_files:
	:return:
	"""
	out = []
	i_files.sort()
	seq = None
	for afile in i_files:
		if afile[0] == '.': continue
		if not isImage( afile): continue

		digits = re.findall('\d+', afile)
		if len(digits) == 0: continue

		digits = digits[-1]

		base = afile[:afile.rfind(digits)]
		padd = len(digits)
		ext  = afile[len(base) + padd :]
		num  = int(digits)

		if seq is not None:
			if seq['base'] == base and seq['padd'] == padd and seq['ext'] == ext and seq['first'] + seq['count'] == num:
				seq['count'] += 1
				seq['last'] = num
				continue
			if seq['count'] > 1:
				out.append(seq)

		seq = dict()
		seq['base']  = base
		seq['padd']  = padd
		seq['ext']   = ext
		seq['first'] = num
		seq['count'] = 1

		if Options.verbose: print(seq)

	if seq is not None and seq['count'] > 1:
		out.append(seq)

	return out

def processShot( i_path):
	"""Initial finction to process given path.
	Assuming that the path is some shot root.
	:param i_path:
	:return:
	"""
	out = dict()
	out['path'] = i_path
	if not os.path.isdir( i_path):
		out['error'] = 'Path does not exist.'
		return out

	sequences = []
	frame_first = 0
	frame_last = 0
	for dirpath, dirnames, filenames in os.walk( i_path):
		if os.path.basename( dirpath)[0] == '.': continue

		seqs = scanSequences( filenames)
		for seq in seqs:

			seq['base'] = os.path.join( dirpath, seq['base'])

			if len(sequences) == 0:
				frame_first = seq['first']
				frame_last = seq['last']
			if seq['first'] < frame_first: frame_first = seq['first']
			if seq['last']  > frame_last:  frame_last  = seq['last']
			
			sequences.append( seq)

	out['name'] = os.path.basename( i_path)
	out['sequences'] = sorted( sequences, key=lambda seq: seq['base'])
	out['sequences_count'] = len(sequences)
	out['frame_first'] = frame_first
	out['frame_last'] = frame_last
	out['fps'] = Options.fps

	return out

def createNukeBackdrop( i_name, i_y, i_w):
	""" Create Nuke 'Backdrop' node
	:param i_name:
	:param i_x:
	:param i_y:
	:return:
	"""
	bd = 'BackdropNode {'
	bd += '\nname %s' % i_name
	bd += '\nxpos -20'
	bd += '\nypos %d' % (i_y - 20)
	bd += '\nbdwidth %d' % (i_w + 20)
	bd += '\nbdheight 120'
	bd += '\n}'
	return bd

def processNuke( io_shot):
	""" Process io_shot to create Nuke script:
	:param io_shot:
	:return:
	"""
	io_shot['file'] = io_shot['path']
	io_shot['file'] = os.path.join( io_shot['file'], Options.output)
	io_shot['file'] = os.path.join( io_shot['file'], 'nuke')
	io_shot['file'] = os.path.join( io_shot['file'], io_shot['name'])
	io_shot['file'] += '_v000.nk'

	io_shot['nuke_name'] = io_shot['file'].replace('\\','/')
	io_shot['nuke_path'] = io_shot['path'].replace('\\','/')

	io_shot['data'] = ''
	src = None
	x = 0
	y = 0
	for seq in io_shot['sequences']:

		filename = seq['base'] + '#'*seq['padd'] + seq['ext']
		filename = os.path.relpath( filename, io_shot['path'])
		filename = filename.replace('\\','/')

		root = filename.split('/')
		if len(root): root = root[0]
		if src is None: src = root
		if src != root:
			io_shot['data'] += '\n' + createNukeBackdrop( src, y, x)
			y += 140
			x = 0
		src = root

		read = 'Read {'
		read += '\ninputs 0'
		read += '\nfile "%s"' % filename
		read += '\nfirst %d' % seq['first']
		read += '\nlast %d' % seq['last']
		read += '\norigset true'
		read += '\nversion 4'
		read += '\nname %s' % ('R_' + os.path.basename(seq['base']).strip(' _.!'))
		read += '\nxpos %d' % x
		read += '\nypos %d' % y
		read += '\n}'

		io_shot['data'] += '\n' + read
		x += 100

	if io_shot['sequences_count']:
		io_shot['data'] += '\n' + createNukeBackdrop( src, y, x)

for path in Args:
	path = os.path.normpath( path)
	Out.append( processShot( path))

if Options.soft == '':
	print(json.dumps({'shot_process': Out}, indent=4))
	sys.exit(0)

for shot in Out:

	if Options.soft == 'nuke': processNuke( shot)

	if not 'file' in shot: continue

	if not Options.debug:
		folder = os.path.dirname(shot['file'])
		if not os.path.isdir(folder):
			try:
				os.makedirs(folder)
			except:
				Out['error'] = 'Can\'t create folder: ' + folder
				continue
		try:
			file = open( shot['file'],'w')
		except:
			Out['error'] = 'Can\'t create file: ' + shot['file']
			continue

	data = TemplateFileData.replace('%','%%')
	data = data.replace('@(','%(')
	data = data % shot
	data = data + shot['data']

	if Options.verbose:
		print(data)

	if not Options.debug:
		file.write(data)
		file.close()

	if Options.run == '': continue

	cmd = '%s "%s" &' % (Options.run, shot['file'])

	if Options.verbose:
		print(cmd)

	os.chdir(shot['path'])

	if not Options.debug:
		os.system(cmd)

print(json.dumps({'shot_process': Out}, indent=4))

