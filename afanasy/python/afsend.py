#!/usr/bin/env python
# -*- coding: utf-8 -*-

import afnetwork

import time, os, sys

from optparse import OptionParser

Parser = OptionParser(usage="%prog [options] output\nType \"%prog -h\" for help", version="%prog 1.0")
Parser.add_option('-c', '--cycles',  dest='cycles',  type='int',          default=1,     help='Cycles for operation repeat')
Parser.add_option('-t', '--time',    dest='time',    type='float',        default=-1,    help='Sleep time between cycles')
Parser.add_option('-V', '--verbose', dest='verbose', action='store_true', default=False, help='Verbose mode')

Options, Args = Parser.parse_args()

if len(Args) < 1:
	print('Send file to Afanasy server.')
	print('Usage: %s file' % sys.argv[0])
	sys.exit(1)

File = Args[0]

if not os.path.isfile(File):
	print('File not found:')
	print(File)
	sys.exit(1)

with open(File, 'rb') as f:
	data = f.read(os.path.getsize(File))

for i in range(0, Options.cycles):

	status, answer = afnetwork.sendServer(data, Options.verbose)

	if not status:
		sys.exit(1)

	if answer:
		print(answer)

	if Options.time > 0:
		time.sleep(Options.time)

