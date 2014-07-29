#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import re
import os
import signal
import sys

from optparse import OptionParser

ImgTypes = ['jpg','png','exr','dpx','tif','tiff','cin','tga']
MovTypes = ['mov','avi','mpg','mpeg','mp4']

Parser = OptionParser(
	usage="%prog [options]\n\tType \"%prog -h\" for help",
	version="%prog 1.0"
)

Parser.add_option('-f', '--fps',      dest='fps',      type='int',          default=24,     help='Frames per second')
Parser.add_option('-o', '--output',   dest='output',   type='string',       default='WORK', help='Software output')
Parser.add_option('-s', '--soft',     dest='soft',     type='string',       default='',     help='Software type')
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

	if seq is not None and seq['count'] > 1:
		out.append(seq)

	return out

def processShot( i_path):
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
	out['sequences'] = sequences
	out['sequences_count'] = len(sequences)
	out['frame_first'] = frame_first
	out['frame_last'] = frame_last

	return out

def createNuke( shot):
	if shot['sequences_count'] == 0: return

	out = dict()
	out['data'] = ''

	x = 0
	y = 0
	for seq in shot['sequences']:
		filename = seq['base'] + '#'*seq['padd'] + seq['ext']

		read = 'Read {'
		read += '\ninputs 0'
		read += '\nfile "%s"' % filename
		read += '\nfirst %d' % seq['first']
		read += '\nlast %d' % seq['last']
		read += '\norigset true'
		read += '\nversion 4'
		read += '\nname %s' % os.path.basename(seq['base'])
		read += '\nxpos %d' % x
		read += '\nypos %d' % y
		read += '\n}'

		out['data'] += '\n' + read
		x += 100

	out['file'] = shot['path']
	out['file'] = os.path.join( out['file'], Options.output)
	out['file'] = os.path.join( out['file'], 'nuke')
	out['file'] = os.path.join( out['file'], shot['name'])
	out['file'] += '.v000.nk'
	shot['nuke'] = out['file']

	return out

for path in Args:
	path = os.path.normpath( path)
	Out.append( processShot( path))

files = []
if Options.soft != '':
	for shot in Out:
		if Options.soft == 'nuke':
			out = createNuke( shot)
			if out: files.append(out)

for afile in files:
	print(afile['file'])
	print(afile['data'])

print(json.dumps({'shot_process': Out}, indent=4))

