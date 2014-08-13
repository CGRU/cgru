#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import subprocess
import sys


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
if Options.output is None: errorExit('Output is not specified.')

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


Cmd = ['7za','a','-y','-r','-t7z','-x!.*',Options.output,Options.input]

print(' '.join(Cmd))

if Options.debug:
	sys.exit(0)

Process = subprocess.Popen( Cmd, shell=False, stdout=subprocess.PIPE)

while True:
	Process.stdout.flush()
	data = Process.stdout.readline()
	if data is None: break
	if len(data) < 1: break

	sys.stdout.write(data)

	if data.find('Compressing ') != -1 and FilesTotal:
		data = data[12:].strip()
		if data not in Files:
			print('PROGRESS: %d%%' % int( 100.0 * len(Files) / FilesTotal))
			Files.append( data)

	sys.stdout.flush()

