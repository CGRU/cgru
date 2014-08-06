#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
import shutil
import signal
import sys
import time

import af

from optparse import OptionParser

Extensions = ['jpg', 'png', 'dpx']
TmpFiles = 'img.%07d.jpg'

Parser = OptionParser(
	usage="%prog [options] inputs\n"
		  "   Type \"%prog -h\" for help",
	version="%prog 1.0"
)

Parser.add_option('-p', '--padding',      dest='padding',      type  ='int',    default=3,           help='Version padding')
Parser.add_option('-i', '--inputs',       dest='inputs',       type  ='string', default='RESULT/JPG',help='Inputs')
Parser.add_option('-d', '--dest',         dest='dest',         type  ='string', default='',          help='Destination')
Parser.add_option(      '--ftp',          dest='ftp'      ,    type  ='string', default=None,        help='FTP Server')
Parser.add_option(      '--ftpuser',      dest='ftpuser',      type  ='string', default=None,        help='FTP User')
Parser.add_option(      '--ftppass',      dest='ftppass',      type  ='string', default=None,        help='FTP Password')
Parser.add_option(      '--afuser',       dest='afuser',       type  ='string', default=None,        help='Afanasy user name')
Parser.add_option(      '--afservice',    dest='afservice',    type  ='string', default='generic',   help='Afanasy service')
Parser.add_option(      '--afmaxtasks',   dest='afmaxtasks',   type  ='int',    default=5,           help='Afanasy max tasks')
Parser.add_option(      '--afcapacity',   dest='afcapacity',   type  ='int',    default=0,           help='Afanasy capacity')
Parser.add_option('-s', '--skipexisting', dest='skipexisting', action='store_true', default=False,   help='Skip existing folders')
Parser.add_option('-e', '--skiperrors',   dest='skiperrors',   action='store_true', default=False,   help='Skip error folders')
Parser.add_option('-t', '--testonly',     dest='testonly',     action='store_true', default=False,   help='Test input only')
Parser.add_option('-V', '--verbose',      dest='verbose',      action='store_true', default=False,   help='Verbose mode')

(Options, args) = Parser.parse_args()

print('{"put":[')


def errExit(i_msg):
	print('{"error":"%s"},' % i_msg)
	print('{"status":"error"}]}')
	sys.exit(1)


def interrupt(signum, frame):
	errExit('Interrupt received')


signal.signal(signal.SIGTERM, interrupt)
signal.signal(signal.SIGABRT, interrupt)
signal.signal(signal.SIGINT, interrupt)

SimilarCharactrers = ' .-()[]{}!'


def simiralName(i_name):
	name = i_name.lower()
	for c in SimilarCharactrers:
		name = name.replace(c, '_')
	return name


if len(args) < 1:
	errExit('Not enough arguments provided.')

if Options.dest == '':
	errExit('Destination is not specified')

Inputs = Options.inputs.split(',')
Sources = args

if not Options.testonly and Options.ftp is None:
	if not os.path.isdir(Options.dest):
		errExit('Destination folder does not exist:\n' + Options.dest)

commands = []
task_names = []
CmdPut = os.environ['CGRU_LOCATION'] + '/utilities/put.py'
CmdPut = 'python "%s"' % os.path.normpath(CmdPut)
CmdPut += ' -d "%s"' % Options.dest
JobName = 'PUT ' + Options.dest
if Options.ftp is not None:
	CmdPut += ' --ftp "%s"' % Options.ftp
	if Options.ftpuser is not None: CmdPut += ' --ftpuser "%s"' % Options.ftpuser
	if Options.ftppass is not None: CmdPut += ' --ftppass "%s"' % Options.ftppass
	JobName = 'FTP PUT ' + Options.dest

for src in Sources:

	folder = None
	version = None
	name = os.path.basename(src)

	for inp in Inputs:
		inp = os.path.join(src, inp)
		if not os.path.isdir(inp):
			continue

		for item in os.listdir(inp):
			if item[0] in '._':
				continue

			if not os.path.isdir(os.path.join(inp, item)):
				continue

			ver = simiralName(item).replace(simiralName(name), '').strip('!_-. ')

			ver_digits = re.findall(r'\d+', ver)
			if len(ver_digits):
				ver_digits = ver_digits[0]
				ver_number = int(ver_digits)
				ver_number = ('%0' + str(Options.padding) + 'd') % ver_number
				ver = ver.replace(ver_digits, ver_number, 1)

			if len(ver) and ver[0].isdigit():
				ver = 'v' + ver

			if version is not None:
				if version >= ver:
					continue
			version = ver

			folder = os.path.join(inp, item)

	if folder is None:
		if Options.skiperrors:
			print('{"error":"%s"},' % src)
			continue
		else:
			errExit('Input not found for: %s' % src)

	if version == '':
		version = ('v%0' + str(Options.padding) + 'd') % 0
	name += '_' + version

	dest = os.path.join(Options.dest, name)
	skipexisting = False
	skipexisting_str = 'false'
	if os.path.isdir(dest) and Options.skipexisting:
		skipexisting = True
		skipexisting_str = 'true'

	print(
		'{"src":"%s","name":"%s","version":"%s","dst":"%s","skipexisting":%s},'
		% (folder, name, version, dest, skipexisting_str)
	)

	if skipexisting:
		continue

	cmd = CmdPut
	cmd += ' -s "%s"' % folder
	cmd += ' -d "%s"' % Options.dest
	cmd += ' -n "%s"' % name

	commands.append(cmd)
	task_names.append(name)

	if Options.verbose:
		print( cmd)

print('{"progress":"%d sequences found"},' % len(commands))

job = af.Job( JobName)
if Options.afuser is not None: job.setUserName(Options.afuser)
job.setMaxRunningTasks(Options.afmaxtasks)
job.setMaxRunTasksPerHost(1)

block = af.Block('put', Options.afservice)
counter = 0
for cmd in commands:
	task = af.Task(task_names[counter])
	task.setCommand(cmd)
	block.tasks.append(task)
	counter += 1

if Options.afcapacity > 0:
	block.setCapacity(Options.afcapacity)

job.blocks.append(block)

if not Options.testonly:
	if not job.send():
		errExit('Can`t send job to server.')

print('{"status":"success"}]}')
