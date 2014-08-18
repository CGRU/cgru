#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
import subprocess
import sys
import time

from optparse import OptionParser
Parser = OptionParser(usage="%prog [Options]\nType \"%prog -h\" for help", version="%prog 1.0")
Parser.add_option('-i', '--input',   dest='input',    type  ='string',     default=None,  help='Input')
Parser.add_option('-o', '--output',  dest='output',   type  ='string',     default=None,  help='Output')
Parser.add_option('-V', '--verbose', dest='verbose',  action='store_true', default=False, help='Verbose mode')
Parser.add_option('-D', '--debug',   dest='debug',    action='store_true', default=False, help='Debug mode')
Options, Args = Parser.parse_args()

def errorExit(i_err):
	print('ERROR: %s' % i_err)
	sys.exit(1)

if Options.input is None: errorExit('Input is not specified.')
if not os.path.isfile(Options.input): errorExit('Input is not a file.')

Name,Ext = os.path.splitext(Options.input)
Ext = Ext.strip('.')

FilesTotal = 0
Files = []
SizeTotal = 0
Size = 0

Cmd = None
Key = None
CmdList = None

if Ext == 'zip':
	CmdList = ['unzip','-l', Options.input]
	Cmd = ['unzip','-o']
	if Options.output is not None: Cmd.extend(['-d', Options.output])
	Cmd.append( Options.input)
	Key = 'inflating: '
elif Ext == '7z':
	CmdList = ['7za','l', Options.input]
	Cmd = ['7za','x','-y']
	if Options.output is not None: Cmd.extend(['-o', Options.output])
	Cmd.append( Options.input)
	Key = 'Extracting '

if Cmd is None:
	errorExit('Unsupported arvhive type "%s"' % Options.type)

print(' '.join(CmdList))

Process = subprocess.Popen( CmdList, shell=False, stdout=subprocess.PIPE)

while True:
	data = Process.stdout.readline()
	if data is None: break
	if len(data) < 1: break

	sys.stdout.write(data)
	sys.stdout.flush()

	if Ext == 'zip':
		data = re.findall('\S+', data)
		if len( data) != 3: continue
		if data[-1] != 'files': continue
		SizeTotal = int(data[0])
		FilesTotal =  int(data[1])
	elif Ext == '7z':
		data = re.findall('\S+', data)
		if len( data) != 6: continue
		if data[3] != 'files,': continue
		SizeTotal = int(data[0])
		FilesTotal =  int(data[2])

print('Files Total: %d' % FilesTotal)
print('Size Total: %d' % SizeTotal)

print(' '.join(Cmd))

if Options.debug:
	sys.exit(0)

Process = subprocess.Popen( Cmd, shell=False, stdout=subprocess.PIPE)

while True:
	data = Process.stdout.readline()
	if data is None: break
	if len(data) < 1: break

	sys.stdout.write(data)

	if data.find(Key) != -1 and FilesTotal:
		if data not in Files:
			print('PROGRESS: %d%%' % int( 100.0 * len(Files) / FilesTotal))
			Files.append( data)

	sys.stdout.flush()

