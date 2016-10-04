#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import re
import signal
import sys

from optparse import OptionParser

Parser = OptionParser(
	usage="%prog [options] paths\n"
		  "   Type \"%prog -h\" for help",
	version="%prog 1.0"
)

Parser.add_option('-r', '--respaths',     dest='respaths',     type  ='string',     default='RESULT/JPG,RESULT/TIF,RESULT/DPX')
Parser.add_option('-d', '--dest',         dest='dest',         type  ='string',     default=None,  help='Destination')
Parser.add_option('-p', '--padding',      dest='padding',      type  ='int',        default=3,     help='Version padding')
Parser.add_option('-s', '--skipcheck',    dest='skipcheck',    action='store_true', default=False, help='Skip destination check')
Parser.add_option('-e', '--skiperrors',   dest='skiperrors',   action='store_true', default=False, help='Skip error folders')
Parser.add_option('-V', '--verbose',      dest='verbose',      action='store_true', default=False, help='Verbose mode')

(Options, args) = Parser.parse_args()

Out = dict()
Out['status'] = 'success'
Results = []
Out['results'] = Results

def Output():
	json.dump({"find_results":Out}, sys.stdout, indent=1)
	sys.stdout.write('\n')

def errExit(i_msg):
	Out['error'] = i_msg
	Out['status'] = 'error'
	Output()
	sys.exit(1)

def interrupt(signum, frame):
	errExit('Interrupt received')

signal.signal(signal.SIGTERM, interrupt)
signal.signal(signal.SIGABRT, interrupt)
signal.signal(signal.SIGINT, interrupt)

if len(args) < 1:
	errExit('Not enough arguments provided.')

DestFiles = None
if Options.dest is not None:
	Out['dest'] = Options.dest
	if not Options.skipcheck:
		if not os.path.isdir( Options.dest):
			errExit('Destination folder does not exist: ' + Options.dest)
		DestFiles = os.listdir( Options.dest)

ResPaths = Options.respaths.split(',')

SimilarCharactrers = ' .-()[]{}!'
def simiralName(i_name):
	name = i_name.lower()
	for c in SimilarCharactrers:
		name = name.replace(c, '_')
	return name

for src in args:

	result = dict()
	result['src'] = None

	version = None
	name = os.path.basename(src)

	for res in ResPaths:
		respath = os.path.join(src, res)
		if not os.path.isdir(respath):
			continue

		for item in os.listdir(respath):
			if item[0] in '._':
				continue

			if not os.path.isdir(os.path.join(respath, item)):
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

			result['src'] = os.path.join(respath, item)
			result['respath'] = res

	if result['src'] is None:
		result['error'] = 'Not founded'
		if not Options.skiperrors:
			errExit('Input not found for: %s' % src)

	if version == '' or version is None:
		version = ('v%0' + str(Options.padding) + 'd') % 0
	name += '_' + version

	result['name'] = name
	result['version'] = version
	result['asset'] = src

	if Options.dest is not None:
		result['dest'] = os.path.join(Options.dest, name)
		if DestFiles is not None:
			for afile in DestFiles:
				if afile.find( result['name']) == 0:
					result['exist'] = True
					break

	Results.append(result)


Out['info'] = "%d sequences found" % len(Results)

Output()

