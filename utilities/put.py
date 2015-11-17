#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
import signal
import sys
import subprocess
import time
import traceback

from optparse import OptionParser
Parser = OptionParser(usage="%prog [Options] output\nType \"%prog -h\" for help", version="%prog 1.0")
Parser.add_option('-s', '--source',  dest='source',   type  ='string',     default='',    help='Source')
Parser.add_option('-d', '--dest',    dest='dest',     type  ='string',     default='',    help='Destination')
Parser.add_option('-n', '--name',    dest='name',     type  ='string',     default='',    help='Name')
Parser.add_option('-r', '--rsync',   dest='rsync',    action='store_true', default=False, help='Use rsync')
Parser.add_option(      '--ftp',     dest='ftp',      type  ='string',     default=None,  help='FTP server')
Parser.add_option(      '--ftpuser', dest='ftpuser',  type  ='string',     default=None,  help='FTP user name')
Parser.add_option(      '--ftppass', dest='ftppass',  type  ='string',     default=None,  help='FTP password')
Parser.add_option('-V', '--verbose', dest='verbose',  action='store_true', default=False, help='Verbose mode')
Parser.add_option('-D', '--debug',   dest='debug',    action='store_true', default=False, help='Debug mode')
Options, Args = Parser.parse_args()

if Options.debug:
	Options.verbose = True

os.umask(0000)


def errorExit(i_msg=None):
	if i_msg:
		print('Error: ' + i_msg)
	if sys.exc_info()[1]:
		traceback.print_exc(file=sys.stdout)
	sys.exit(1)

def interrupt(signum, frame):
	errorExit('Interrupt received')

signal.signal(signal.SIGTERM, interrupt)
signal.signal(signal.SIGABRT, interrupt)
signal.signal(signal.SIGINT,  interrupt)

def makeDir(i_folder):
	if len(i_folder) == 0:
		return

	if os.path.isdir(i_folder):
		return

	print('Creating folder:\n' + i_folder)

	if Options.debug:
		return

	try:
		os.makedirs(i_folder)
	except:
		errorExit('Can`t create folder "%s"' % i_folder)


if Options.source == '':
	errorExit('Source is not specified')

if not os.path.exists(Options.source):
	errorExit('Source does not exist:\n' + Options.source)

if Options.dest == '':
	errorExit('Destination is not specified')

if Options.name == '':
	Options.name = os.path.basename( Options.source)

if Options.ftp is None:
	if not os.path.isdir(Options.dest):
		errorExit('Destination folder does not exist:\n' + Options.dest)

Result = os.path.join(Options.dest, Options.name)
SizeTotal = 0
Files = []

if os.path.isdir(Options.source):
	for dirpath, dirnames, filenames in os.walk(Options.source):
		for f in filenames:
			SizeTotal += os.path.getsize(os.path.join(dirpath, f))

	if Options.ftp is None:
		makeDir(Result)
		allfiles = os.listdir(Options.source)
		for afile in allfiles:
			if afile[0] == '.':
				continue
			afile = os.path.join(Options.source, afile)
			if os.path.isfile(afile):
				Files.append(afile)
			elif os.path.isdir(afile):
				Files.append(afile)

		Files.sort()

elif os.path.isfile( Options.source):
	SizeTotal = os.path.getsize( Options.source)
	Files = [Options.source]

else:
	errorExit('Invalid source type')

if SizeTotal == 0:
	errorExit('Source is empty')


CmdFtp = ['ncftpput','-v','-R']
if Options.ftpuser is not None: CmdFtp.extend(['-u',Options.ftpuser])
if Options.ftppass is not None: CmdFtp.extend(['-p',Options.ftppass])
CmdFtp.extend([Options.ftp, Options.dest, Options.source])


CmdCPFile = 'cp -p "%s" "%s"'
CmdCPDir = 'cp -rp "%s" "%s"'
if sys.platform.find('win') == 0:
	CmdCPFile = 'COPY "%s" "%s"'
	CmdCPDir = 'XCOPY "%s" "%s" /YSIR'
if Options.rsync:
	CmdCPFile = 'rsync -avP "%s" "%s"'
	CmdCPDir = 'rsync -avP "%s" "%s"'



print('Source:      %s' % Options.source)
print('Destination: %s' % Options.dest)
if Options.ftp is None:
	print('Result:      %s' % Result)
else:
	print('FTP Server:  %s' % Options.ftp)
print('Total Size:  %d' % SizeTotal)

if Options.debug: sys.exit(0)


#
# Copy files:
#
if Options.ftp is None:
	i = 0
	for afile in Files:
		if Options.verbose:
			print(os.path.basename(afile))

		dest = Result
		if not Options.rsync and os.path.isdir(Options.source):
			dest = os.path.join(Result, os.path.basename(afile))

		Copy = CmdCPFile
		if os.path.isdir(afile):
			Copy = CmdCPDir

		cmd = Copy % (afile, dest)

		print(cmd)
		sys.stdout.flush()

		if not Options.debug:
			status = os.system(cmd)
			if status != 0:
				sys.exit(status)

		if not Options.rsync:
			print('PROGRESS: %d%%' % int(100.0 * (i + 1.0) / len(Files)))

		if i == int(len(Files)/2):
			print('@IMAGE@' + afile)

		sys.stdout.flush()

		i += 1

	sys.exit(0)

#
# FTP put files:
#
try:
	process = subprocess.Popen(CmdFtp, shell=False, stderr=subprocess.PIPE)
except Exception as e:
	print('Command execution error:')
	print(str(e))
	sys.exit(1)


TimeStart = time.time()
SizeCurrent = 0
FileCurrent = None
while True:
	stdout = ''
	data = process.stderr.readline()
	if data is None:
		break
	if len(data) < 1:
		break
	if not isinstance(data, str):
		data = str(data, 'ascii')
	#data = data.replace('\r', '\n')

	if data.find('ncftpput') == 0:
		sys.stdout.write(data)
		sys.stdout.flush()
		continue

	if data.find('ETA:') != -1: continue

	data = data.split('\n')
	if len(data) < 2: continue
	data = data[-2]

	print(data)

	data = data.split(':')
	if len(data) < 2: continue

	if FileCurrent == data[0]: continue
	FileCurrent = data[0]
	data = data[1]

	data = re.findall('\S+', data)
	if len(data) < 2: continue

	size = None
	try:
		size = float(data[0])
	except Exception as e:
		pass
	if size is None: continue

	if size <= 0: continue

	data = data[1]
	if len(data) > 1:
		if data[0] == 'k': size *= 1024
		if data[0] == 'M': size *= 1024*1024

	SizeCurrent += size

	print('PROGRESS: %d%%' % int(100.0*SizeCurrent/SizeTotal))

	speed = time.time()-TimeStart
	if speed > 0:
		speed = int(SizeCurrent/speed)
		b = 'B'
		if speed > 4096:
			speed = int(speed/1024)
			b = 'kB'
		if speed > 4096:
			speed = int(speed/1024)
			b = 'MB'
		print('SPEED: %d %s/s' % (speed,b))

	sys.stdout.flush()

if SizeCurrent == 0:
	errorExit('No files transfered')

b = 'B'
if SizeTotal > 4096:
	SizeTotal = int(SizeTotal/1024)
	b = 'kB'
if SizeTotal > 4096:
	SizeTotal = int(SizeTotal/1024)
	b = 'MB'
if SizeTotal > 4096:
	SizeTotal = int(SizeTotal/1024)
	b = 'GB'

print('Transfered: %d %s' % (SizeTotal,b))

