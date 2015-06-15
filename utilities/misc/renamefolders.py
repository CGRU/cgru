#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
import sys

from optparse import OptionParser
parser = OptionParser(usage="%prog [options] path\ntype \"%prog -h\" for help", version="%prog 1.  0")
parser.add_option('-T', '--test' ,   dest='test',    action='store_true', default=False, help='Test mode.')
parser.add_option('-V', '--verbose', dest='verbose', action='store_true', default=False, help='Verbose.')

Options, Args = parser.parse_args()

Root = os.getcwd()
if len(Args) > 0:
	Root = Args[0]

if not os.path.isdir( Root):
	print('ERROR: Root folder does not exist:\n' + Root)
	sys.exit(1)

if Options.verbose:
	print('Root = "%s"' % Root)

ListDir = os.listdir(Root)
ListDir.sort()

OLD = []
NEW = []

for fname in ListDir:

	fpath = os.path.join(Root, fname)

	if not os.path.isdir(fpath):
		continue

	items = os.listdir(fpath)

	for item in items:
		if item[0] == '.':
			continue

		name, ext = os.path.splitext(item)

		if len(ext) == 0:
			continue

		name, ext = os.path.splitext(name)

		if len(ext) == 0:
			continue

		OLD.append(fpath)
		NEW.append( os.path.join( Root, name))

		break

for i in range(0, len(OLD)):
	if Options.verbose:
		print('"%s" -> "%s"' % (os.path.basename(OLD[i]), os.path.basename(NEW[i])))
	if not Options.test:
		os.rename( OLD[i], NEW[i])

