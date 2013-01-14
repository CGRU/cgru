#!/usr/bin/env python

import re, os, sys

from optparse import OptionParser

Parser = OptionParser( usage="%prog [options]\ntype \"%prog -h\" for help", version="%prog 1.0")

Parser.add_option('-r', '--resolution', dest='resolution', type  ='string',     default='160x90',
	help='Resolution')
Parser.add_option('-i', '--input',      dest='input',      type  ='string',     default='',
	help='Input image')
Parser.add_option('-o', '--output',     dest='output',     type  ='string',     default='thumbnail.jpg',
	help='Output image')
Parser.add_option('-f', '--force',      dest='force',      action='store_true', default=False,
	help='Force creation, no modification time check.')
Parser.add_option('-V', '--verbose',    dest='verbose',    action='store_true', default=False,
	help='Verbose mode.')
Parser.add_option('-D', '--debug',      dest='debug',      action='store_true', default=False,
	help='Debug mode.')

(Options, Args) = Parser.parse_args()

if Options.input == '': Parser.error('Input image not specified.')

image = None

if Options.input.find(',') != -1 or os.path.isdir( Options.input):
	folders = [Options.input]
	if folders[0].find(',') != -1:
		folders = folders[0].split(',')
	paths = dict()
	for folder in folders:
		if Options.verbose: print('Scanning folder "%s":' % folder)
		for root, dirs, files in os.walk( folder):
			if len( files) == 0: continue
			images = []
			for afile in files:
				if re.search( r'd\.', afile):
					images.append( afile)
			if len( images) == 0: continue
			print( images)
			print(root+': '+str())
			paths[int(os.path.getmtime(root))] = root
	print( paths)
else:
	if not os.path.isfile( Options.input): Parser.error('Can`t fide input file:\n' + str( Options.input))

if Options.verbose: print('Input: "%s"' % image)

