#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
import subprocess
import sys
import time

import cgruutils

from optparse import OptionParser
Parser = OptionParser(usage="%prog [Options]\nType \"%prog -h\" for help", version="%prog 1.0")
Parser.add_option('-i', '--input',   dest='input',    type  ='string',     default=None,  help='Input')
Parser.add_option('-o', '--output',  dest='output',   type  ='string',     default=None,  help='Output')
Parser.add_option('-u', '--thumbsec',dest='thumbsec', type  ='int',        default=None,  help='Output thumbnail path frequency')
Parser.add_option('-V', '--verbose', dest='verbose',  action='store_true', default=False, help='Verbose mode')
Parser.add_option('-D', '--debug',   dest='debug',    action='store_true', default=False, help='Debug mode')
Options, Args = Parser.parse_args()

os.umask(0000)

def errorExit(i_err):
	print('ERROR: %s' % i_err)
	sys.exit(1)

if Options.input is None: errorExit('Input is not specified.')
if not os.path.isfile(Options.input): errorExit('Input is not a file.')

Name,Ext = os.path.splitext(Options.input)
Ext = Ext.strip('.').lower()

FilesTotal = 0
Files = []
SizeTotal = 0
Size = 0

Cmd = None
Key = None
CmdPre = None
CmdList = None

if Ext == 'zip':
	parts, tmp = os.path.splitext( Options.input)
	parts = parts + '.z01'
	print(parts)
	if os.path.isfile( parts):
		CmdPre = 'zip -F "%s" --output "%s"' % (Options.input,(Options.input+'.zip'))
		Options.input += '.zip'
	CmdList = ['unzip','-l', Options.input]
	Cmd = ['unzip','-o']
	if Options.output is not None: Cmd.extend(['-d', Options.output])
	Cmd.append( Options.input)
	Key = 'inflating: '
elif Ext == 'rar':
	CmdList = ['unrar','l', Options.input]
	Cmd = ['unrar','x','-r','-o+','-y', Options.input]
	if Options.output is not None: Cmd.append( Options.output)
	Key = 'Extracting '
elif Ext == '7z' or Ext == '001':
	CmdList = ['7za','l', Options.input]
	Cmd = ['7za','x','-y']
	if Options.output is not None: Cmd.extend(['-o', Options.output])
	Cmd.append( Options.input)
	Key = 'Extracting '

if Cmd is None:
	errorExit('Unsupported arvhive type "%s"' % Ext)

if CmdPre:
	print( CmdPre)
	os.system( CmdPre)

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
		SizeTotal  = int(data[0])
		FilesTotal = int(data[1])
	elif Ext == 'rar':
		data = re.findall('\S+', data)
		if len( data) != 5: continue
		try:
			SizeTotal  += int(data[1])
			FilesTotal += 1
		except:
			continue
	elif Ext == '7z':
		data = re.findall('\S+', data)
		if len( data) != 6: continue
		if data[3] != 'files,': continue
		SizeTotal  = int(data[0])
		FilesTotal = int(data[2])

print('Files Total: %d' % FilesTotal)
print('Size Total: %d' % SizeTotal)

print(' '.join(Cmd))

returncode = Process.wait()
if returncode != 0:
	sys.exit(returncode)

if Options.debug:
	sys.exit(0)


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

