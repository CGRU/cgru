#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
import sys

invalid_characters = '<>:;![]()$%^&*`\\|/?"\''
replace_character = '_'

from optparse import OptionParser
parser = OptionParser(usage="%prog [options] path\ntype \"%prog -h\" for help", version="%prog 1.  0")
parser.add_option('-r', '--regexp',  dest='regexp',  type='string',       default=None,  help='Match regular expression.')
parser.add_option('-s', '--skipre',  dest='skipre',  type='string',       default=None,  help='Skip regular expression.')
parser.add_option('-d', '--dirs',    dest='dirs',    action='store_true', default=False, help='Check directories names.')
parser.add_option('-V', '--verbose', dest='verbose', action='store_true', default=False, help='Verbose.')

Options, Args = parser.parse_args()

def errorExit(i_err):
	print('ERROR: %s' % i_err)
	sys.exit(1)

if len( Args) < 1:
	errorExit('Path is not specified.')

RegExp = None
if Options.regexp is not None:
	RegExp = re.compile( Options.regexp)
SkipRE = None
if Options.skipre is not None:
	SkipRE = re.compile( Options.skipre)

Files = []
FilesPaths = []
Folders = []
FoldersPaths = []

DubFilesCount = 0
DubFoldersCount = 0

Root = Args[0]
RootDir = os.path.dirname(Root)
Roots = []
for adir in os.listdir(RootDir):
	if os.path.join( RootDir, adir).find( Root) == 0:
		Roots.append( os.path.join( RootDir, adir))
Roots.sort()

for rdir in Roots:
	for dirpath, dirnames, filenames in os.walk(rdir):

		dirbase = os.path.basename( dirpath)
		if len( dirbase) and dirbase[0] == '.': continue

		if SkipRE is not None:
			if SkipRE.match( dirpath) is not None: continue

		for afile in filenames:
			if RegExp is not None:
				if RegExp.match( afile) is None: continue

			if SkipRE is not None:
				if SkipRE.match( afile) is not None: continue

			if Options.verbose: print( afile)

			if afile in Files:
				DubFilesCount += 1
				print('Dublicate File[%d]:' % DubFilesCount)
				i = Files.index( afile)
				pre_file = os.path.join( FilesPaths[i], Files[i])
				new_file = os.path.join( dirpath, afile)
				print('%s (%d bytes)' % (pre_file, os.path.getsize(pre_file)))
				print('%s (%d bytes)' % (new_file, os.path.getsize(new_file)))
			else:
				Files.append( afile)
				FilesPaths.append( dirpath)

		if not Options.dirs: continue

		for adir in dirnames:
			if RegExp is not None:
				if RegExp.match( adir) is None: continue

			if SkipRE is not None:
				if SkipRE.match( adir) is not None: continue

			if Options.verbose: print( adir)

			if adir in Folders:
				DubFoldersCount += 1
				print('Dublicate Folder[%d]:' % DubFoldersCount)
				i = Folders.index( adir)
				print( os.path.join( FoldersPaths[i], Folders[i]))
				print( os.path.join( dirpath, adir))
			else:
				Folders.append( adir)
				FoldersPaths.append( dirpath)

print('Dublicate files count: %d' % DubFilesCount)
if DubFoldersCount:
	print('Dublicate folders count: %d' % DubFoldersCount)

