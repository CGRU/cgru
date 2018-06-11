#!/usr/bin/env python
# -*- coding: utf-8 -*-

import getpass
import json
import os
import signal
import sys

from optparse import OptionParser

Parser = OptionParser(
	usage="%prog [options]\n\tType \"%prog -h\" for help",
	version="%prog 1.0"
)

Parser.add_option('-f', '--folder',   dest='folder',   type='string',       default='WORK', help='"WORK" folder')
Parser.add_option('-s', '--soft',     dest='soft',     type='string',       default='nuke', help='Software type')
Parser.add_option('-e', '--ext',      dest='ext',      type='string',       default='.nk',  help='Software file extension')
Parser.add_option('-r', '--run',      dest='run',      type='string',       default='nuke', help='Software run command')
Parser.add_option('-V', '--verbose',  dest='verbose',  action='store_true', default=False,  help='Verbose mode')
Parser.add_option('-D', '--debug',    dest='debug',    action='store_true', default=False,  help='Debug mode')

Out = []
OutName = 'open_latest'

def errExit(i_msg):
	Out.append({'error': i_msg})
	Out.append({'status': 'error'})
	print(json.dumps({OutName: Out}, indent=4))
	sys.exit(1)


def interrupt(signum, frame):
	errExit('Interrupt received')

signal.signal(signal.SIGTERM, interrupt)
signal.signal(signal.SIGABRT, interrupt)
signal.signal(signal.SIGINT, interrupt)

(Options, Args) = Parser.parse_args()
if Options.debug: Options.verbose = True

if len( Args) == 0:
	errExit('Shot path is not specified.')

SceneFolder = Args[0]
if not os.path.isdir( SceneFolder):
	errExit('Path does not exist.')
os.chdir( SceneFolder)

SceneFile = ''
SceneFolder = os.path.join( SceneFolder, Options.folder)

if Options.soft != '':
	SceneFolder = os.path.join( SceneFolder, Options.soft)

folder_user = os.path.join( SceneFolder, getpass.getuser())
if os.path.isdir( folder_user):
	SceneFolder = folder_user
Out.append({'folder':SceneFolder})

if not os.path.isdir( SceneFolder):
	errExit('No scenes folder.')

allfiles = os.listdir( SceneFolder)
scenes = []
for afile in allfiles:
	if not os.path.isfile(os.path.join(SceneFolder, afile)): continue
	if afile[0] == '.': continue
	# We should skip files with more than one extension
	# Assuming that second extension is a temp render scene
	if afile.lower().find( Options.ext.lower()) == len (afile) - len(Options.ext):
		scenes.append( afile)

if len(scenes) == 0:
	errExit('No scenes founded.')

scenes.sort()
if Options.verbose:
	for afile in scenes:
		print( afile)

SceneFile = os.path.join( SceneFolder, scenes[-1])
cmd = '%s "%s" &' % (Options.run, SceneFile)

Out.append({'file':SceneFile})
Out.append({'cmd':cmd})

print(json.dumps({OutName: Out}, indent=4))

if not Options.debug:
	os.system(cmd)

