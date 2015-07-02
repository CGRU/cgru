#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import re
import shutil
import sys

from cgrusequence import cgruSequence

invalid_characters = '<>:;![]()$%^&*`\\|/?"\''
replace_character = '_'

from optparse import OptionParser
parser = OptionParser(usage="%prog [options] path\ntype \"%prog -h\" for help", version="%prog 1.  0")
parser.add_option('-T', '--test',    dest='test',    action='store_true', default=False, help='Test mode.')
parser.add_option('-V', '--verbose', dest='verbose', action='store_true', default=False, help='Verbose.')

Options, Args = parser.parse_args()

Out = []
Sequences = None

def dumpOut():
	Out.append({'sequences': Sequences})
	print(json.dumps({'findsequences': Out}, indent=4))

def errExit(i_msg):
	Out.append({'error': i_msg})
	Out.append({'status': 'error'})
	dumpOut()
	sys.exit(1)

def sizeof_fmt(num, suffix='B'):
    for unit in ['','K','M','G','T','P','E','Z']:
        if abs(num) < 1024.0:
            return "%3.1f%s%s" % (num, unit, suffix)
        num /= 1024.0
    return "%.1f%s%s" % (num, 'Yi', suffix)

InDir = os.getcwd()
if len(Args): InDir = Args[0]

Sequences = cgruSequence( os.listdir( InDir), Options.verbose)

SeqCount = 0
SeqFiles = 0
SeqSize = 0

for seq in Sequences:
	if not seq['seq']: continue
	SeqCount += 1

	mkdir = os.path.join( InDir, seq['prefix'].strip('._ -()[]'))

	pattern = seq['prefix'] + '%0' + str(seq['padding']) + 'd' + seq['suffix']

	if Options.verbose: print(os.path.join(mkdir,pattern))

	if not Options.test:
		if os.path.isdir(mkdir):
			errExit('Folder "%s" already exits.' % mkdir)
		try:
			os.mkdir(mkdir)
		except:
			pass
		if not os.path.isdir(mkdir):
			errExit('Can`t create "%s" folder.' % mkdir)

	for f in range(seq['first'],seq['last']+1):

		src = os.path.join( InDir, pattern % f)
		dst = os.path.join( mkdir, pattern % f)

		#if Options.verbose: print('%s -> %s' % (src,dst))

		if not os.path.isfile(src):
			errExit('File "%s" does not exit.' % src)
		if os.path.isfile(dst):
			errExit('File "%s" already exits.' % dst)

		SeqFiles += 1
		SeqSize += os.path.getsize(src)

		if Options.test: continue

		try:
			shutil.move( src, dst)
		except:
			pass

		if not os.path.isfile(dst):
			errExit('Can`t move to "%s" file.' % dst)

if not Options.verbose:
	Out.append({'count': SeqCount})
	Out.append({'files': SeqFiles})
	Out.append({'size': SeqSize})
	dumpOut()
else:
	print('Total: Sequences = %d, Files count = %d, Files size = %s' % ( SeqCount, SeqFiles, sizeof_fmt(SeqSize)))

