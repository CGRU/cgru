#!/usr/bin/env python
# -*- coding: utf-8 -*-

import operator
import os
import re
import sys

from optparse import OptionParser
parser = OptionParser(usage="%prog [options] path\ntype \"%prog -h\" for help", version="%prog 1.  0")
parser.add_option('-u', '--upc',  dest='upc',  action='store_true', default=False, help='Uppercase.')
parser.add_option('-z', '--zero', dest='zero', action='store_true', default=False, help='Add a "-0" to the first existing sequence.')
parser.add_option('-r', '--run',  dest='run',  action='store_true', default=False, help='Run rename.')

Options, Args = parser.parse_args()

if Options.run is False:
	parser.print_help()

Root = os.getcwd()
if len(Args) > 0:
	Root = Args[0]

if not os.path.isdir( Root):
	print('ERROR: Root folder does not exist:\n' + Root)
	sys.exit(1)

ListDir = os.listdir(Root)
ListDir.sort()

NAMES = []
OLD = []
NEW = []

for fname in ListDir:

	fpath = os.path.join(Root, fname)

	if not os.path.isdir(fpath):
		continue

	items = os.listdir(fpath)

	name = None
	for name in items:
		# Skip hidden:
		if name[0] == '.':
			continue

		# Skip with no extension:
		name, ext = os.path.splitext(name)
		if len(ext) == 0: continue

		break

	name = re.split('\d+$', name)[0]
	name = name.strip(' -.()[]_')
	if Options.upc: name = name.upper()

	old = fpath
	new = os.path.join( Root, name)

	# Sequence may be already exists:
	if name in NAMES:
		# Add to existing sequence '-0':
		if Options.zero and new in NEW:
			i = NEW.index(new)
			NEW[i] += '-0'

		# Search for a new sequence number:
		n = 1
		new_i = '%s-%d' % (new, n)
		while new_i in NEW:
			new_i = '%s-%d' % (new, n)
			n += 1
		new = new_i
	else:
		NAMES.append(name)

	OLD.append( old)
	NEW.append( new)


ENT = []
for i in range(0, len(OLD)): ENT.append({'old':OLD[i],'new':NEW[i]})
ENT.sort( key=operator.itemgetter('new'))


for i in range(0, len(ENT)):

	old = ENT[i]['old']
	new = ENT[i]['new']

	if new == old:
		print('Skipping: ' + os.path.basename(old))
		continue

	print('"%s" -> "%s"' % (os.path.basename(old), os.path.basename(new)))

	if Options.run:
		os.rename( old, new)

