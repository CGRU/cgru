#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys

import cgrudocs

from optparse import OptionParser
Parser = OptionParser(usage="%prog [Options] output\nType \"%prog -h\" for help", version="%prog 1.0")
Parser.add_option('-V', '--verbose', dest='verbose', action='store_true', default=False, help='Verbose mode')
Parser.add_option('-D', '--debug',   dest='debug',   action='store_true', default=False, help='Debug mode')
Options, Args = Parser.parse_args()

if Options.debug:
	Options.verbose = True

Path = None
if len(Args):
	Path = Args[-1]

cgrudocs.show( Path)

