#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import shutil

from optparse import OptionParser

Parser = OptionParser(
	usage="%prog [options] name\ntype \"%prog -h\" for help",
	version="%prog 1.0"
)

Parser.add_option('-t', '--template', dest='template', type='string', default=None, help='Template folder.')
Parser.add_option('-d', '--dest',     dest='dest',     type='string', default=None, help='Destination folder.')

Options, Args = Parser.parse_args()

Out = dict()

def errExit(i_msg):
	Out['error'] = i_msg
	Out['status'] = 'error'
	print( json.dumps({'copy': Out}, indent=4))
	sys.exit(1)

if len(Args) < 1:
	errorExit('Input is not specified.')

if not os.path.isdir( Options.template):
	errorExit('Input folder does not exit.')

if not os.path.isdir( Options.dest):
	errorExit('Destination folder does not exit.')

Out['copies'] = []
for name in Args:
	dest = os.path.join( Options.dest, name)
	if not os.path.isdir( dest):
		shutil.copytree( Options.template, dest)
	Out['copies'].append( dest)

print( json.dumps({'copy': Out}, indent=4))

