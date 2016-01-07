#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import subprocess
import sys
import time

import cgruutils

from optparse import OptionParser
Parser = OptionParser(usage="%prog [Options]\nType \"%prog -h\" for help", version="%prog 1.0")
Parser.add_option('-i', '--input',   dest='input',    type  ='string',     default=None,  help='Input')
Parser.add_option('-o', '--output',  dest='output',   type  ='string',     default=None,  help='Output')
Parser.add_option('-t', '--type',    dest='type',     type  ='string',     default='zip', help='Type')
Parser.add_option('-s', '--split',   dest='split',    type  ='int',        default=None,  help='Split arhive size(MB)')
Parser.add_option('-u', '--thumbsec',dest='thumbsec', type  ='int',        default=None,  help='Output thumbnail path frequency')
Parser.add_option('-V', '--verbose', dest='verbose',  action='store_true', default=False, help='Verbose mode')
Parser.add_option('-D', '--debug',   dest='debug',    action='store_true', default=False, help='Debug mode')
Options, Args = Parser.parse_args()

def errorExit(i_err):
	print('ERROR: %s' % i_err)
	sys.exit(1)

if Options.input is None: errorExit('Input is not specified.')

if Options.output is None:
	Options.output = Options.input + '.' + Options.type

FilesTotal = 0
Files = []
SizeTotal = 0
Size = 0

if os.path.isdir( Options.input):
	for dirpath, dirnames, filenames in os.walk( Options.input):
		FilesTotal += len(filenames)
		for afile in filenames:
			SizeTotal += os.path.getsize( os.path.join( dirpath, afile))
	print('Files Total: %d' % FilesTotal)
	print('Size Total: %d' % SizeTotal)


if Options.output[-1-len(Options.type):] != '.'+Options.type:
	Options.output += '.' + Options.type

Cmd = None
Key = None
if Options.type == 'zip':
	Cmd = ['zip','-y','-r','-1', Options.output, Options.input,'-x','*/.*']
	if Options.split:
		Cmd.append('-s%dm' % Options.split)
	Cmd.extend([Options.output, Options.input,'-x','*/.*'])
	Key = 'adding: '

elif Options.type == 'rar':
	Cmd = ['rar','a','-y','-r','-o+','-x*/.*/', Options.output, Options.input]
	if Options.split:
		Cmd.append('-v%dm' % Options.split)
	Cmd.extend([Options.output, Options.input])
	Key = 'Adding '

elif Options.type == '7z':
	Cmd = ['7za','a','-y','-r','-t7z','-x!.*']
	if Options.split:
		Cmd.append('-v%dm' % Options.split)
	Cmd.extend([Options.output, Options.input])
	Key = 'Compressing '

if Cmd is None:
	errorExit('Unsupported arvhive type "%s"' % Options.type)

print(' '.join(Cmd))

if Options.debug:
	sys.exit(0)

if os.path.isfile(Options.output):
	print('Deleting exising archive:\n' + Options.output)
	os.remove(Options.output)

Process = subprocess.Popen( Cmd, shell=False, stdout=subprocess.PIPE)

ThumbTime = 0

while True:
	data = Process.stdout.readline()
	if data is None: break
	if len(data) < 1: break

	sys.stdout.write(data)

	if data.find(Key) != -1 and FilesTotal:
		if data not in Files:
			print('PROGRESS: %d%%' % int( 100.0 * len(Files) / FilesTotal))
			Files.append( data)

			curtime = time.time()
			if Options.thumbsec is not None and curtime - ThumbTime > Options.thumbsec:
				data = data[data.find(Key):]
				data = data[len(Key):]
				data = data.strip(' \n\r')
				data = data.split(' ')[0]
				#data = os.path.abspath(data)
				if cgruutils.isImageExt(data) and os.path.isfile( data):
					print('@IMAGE!@' + data)
					ThumbTime = curtime

	sys.stdout.flush()

returncode = Process.wait()
sys.exit(returncode)

