#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys

import cgruconfig
import cgrules
import cgruutils

from optparse import OptionParser
Parser = OptionParser(usage="%prog [Options] output\nType \"%prog -h\" for help", version="%prog 1.0")
Parser.add_option('-s', '--shot',    dest='shot',    action='store_true', default=False, help='Open shot')
Parser.add_option('-V', '--verbose', dest='verbose', action='store_true', default=False, help='Verbose mode')
Parser.add_option('-D', '--debug',   dest='debug',   action='store_true', default=False, help='Debug mode')
Options, Args = Parser.parse_args()

if Options.debug:
	Options.verbose = True

if not 'rules_url' in cgruconfig.VARS:
	print('Rules URL is not defined.')
	sys.exit(1)

Location = None
if len(Args):
	Location = Args[-1]
else:
	cgruutils.webbrowse( cgruconfig.VARS['rules_url'])
	sys.exit(0)

if Options.verbose:
	print('Opening "%s"' % Location)

if Options.shot:
	cgrules.openShot( Location, Options.verbose)
else:
	cgrules.openPath( Location, Options.verbose)

