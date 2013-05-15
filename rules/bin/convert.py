#!/usr/bin/env python

import json, os, sys

from optparse import OptionParser

Parser = OptionParser( usage="%prog [options]\ntype \"%prog -h\" for help", version="%prog 1.0")

Parser.add_option('-r', '--resize',  dest='resize',     type  ='string',     default='', help='Resize')
Parser.add_option('-q', '--quality', dest='quality',    type  ='int',        default=75, help='Quality')
Parser.add_option('-i', '--input',   dest='input',      type  ='string',     default='', help='Input Image')
Parser.add_option('-o', '--output',  dest='output',     type  ='string',     default='', help='Output Image')
Parser.add_option('-V', '--verbose', dest='verbose',    action='store_true', default=False, help='Verbose mode.')
Parser.add_option('-D', '--debug',   dest='debug',      action='store_true', default=False, help='Debug mode.')

(Options, Args) = Parser.parse_args()

out = dict()

def errorExit( i_err):
	out['error'] = i_err
	print( json.dumps( out))
	sys.exit(1)

if Options.input == '': errorExit('Input not specified.')

output = Options.output
if output == '':
	output = Options.input
	output += '.%d' % Options.quality
	if Options.resize != '':
		output += '.%s' % Options.resize
	output += '.jpg'
out['convert'] = output

if not os.path.isfile( Options.input):
	errorExit('Input not founded.')

if os.path.isfile( output):
	errorExit('Convert output already exists: ' + output)

cmd = 'convert'

cmd += ' "%s"' % Options.input

cmd += ' -flatten'

imgtype = Options.input.rfind('.');
if imgtype > 0:
	imgtype = Options.input[imgtype+1:].lower()
	if   imgtype == 'exr': cmd += ' -set colorspace sRGB'
	elif imgtype == 'dpx': cmd += ' -set colorspace Log'
	elif imgtype == 'cin': cmd += ' -set colorspace Log'

if Options.resize != '': cmd += ' -resize %s' % Options.resize

cmd += ' -quality %d' % Options.quality

cmd += ' "%s"' % output

if Options.verbose or Options.debug:
	print( cmd)

if Options.debug:
	print('Debug mode. Exiting...')
	sys.exit( 0)

os.system( cmd)

print( json.dumps( out))

