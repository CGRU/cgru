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
Parser = OptionParser(usage="%prog [Options]\nType \"%prog -h\" for help", version="%prog 1.0")
Parser.add_option('-f', '--ftphost',  dest='ftphost',  type  ='string',     default='',    help='FTP server')
Parser.add_option('-u', '--username', dest='username', type  ='string',     default='',    help='FTP user name')
Parser.add_option('-p', '--password', dest='password', type  ='string',     default='',    help='FTP password')
Parser.add_option('-s', '--source',   dest='source',   type  ='string',     default='',    help='Source')
Parser.add_option('-d', '--dest',     dest='dest',     type  ='string',     default='',    help='Destination')
Parser.add_option('-V', '--verbose',  dest='verbose',  action='store_true', default=False, help='Verbose mode')
Parser.add_option('-D', '--debug',    dest='debug',    action='store_true', default=False, help='Debug mode')
Options, Args = Parser.parse_args()

if Options.debug:
	Options.verbose = True

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

if Options.ftphost == '':
	errorExit('FTP server is not specified')

if Options.source == '':
	errorExit('Source is not specified')

if not os.path.exists(Options.source):
	errorExit('Source does not exist:\n' + Options.source)

if Options.dest == '':
	errorExit('Destination is not specified')

SizeTotal = 0

for dirpath, dirnames, filenames in os.walk(Options.source):
	for f in filenames:
		SizeTotal += os.path.getsize(os.path.join(dirpath, f))

if SizeTotal == 0:
	errorExit('Source is empty')

Cmd = ['ncftpput','-v','-R']
if Options.username != '': Cmd.extend(['-u',Options.username])
if Options.password != '': Cmd.extend(['-p',Options.password])
Cmd.extend([Options.ftphost, Options.dest, Options.source])

if Options.verbose:
	print('FTP Server:  %s' % Options.ftphost)
	print('Source:      %s' % Options.source)
	print('Destination: %s' % Options.dest)
	print('Total Size:  %d' % SizeTotal)
	print(' '.join(Cmd))

if Options.debug: sys.exit(0)

try:
	process = subprocess.Popen(Cmd, shell=False, stderr=subprocess.PIPE)
except Exception as e:
	print('Command execution error:')
	print(str(e))
	sys.exit(1)


# ncftpput -v -R localhost /incoming ../afanasy

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

