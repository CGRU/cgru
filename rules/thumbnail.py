#!/usr/bin/env python

import re, os, sys

Extesions = ['dpx','exr','jpg','jpeg','png','tif']

from optparse import OptionParser

Parser = OptionParser( usage="%prog [options]\ntype \"%prog -h\" for help", version="%prog 1.0")

Parser.add_option('-x', '--xres',    dest='xres',       type  ='int',        default=160,
	help='X Resolution')
Parser.add_option('-y', '--yres',    dest='yres',       type  ='int',        default=90,
	help='Y Resolution')
Parser.add_option('-n', '--number',  dest='number',     type  ='int',        default=3,
	help='Number of images')
Parser.add_option('-i', '--input',   dest='input',      type  ='string',     default='',
	help='Input image')
Parser.add_option('-o', '--output',  dest='output',     type  ='string',     default='thumbnail.jpg',
	help='Output image')
Parser.add_option('-f', '--force',   dest='force',      action='store_true', default=False,
	help='Force creation, no modification time check.')
Parser.add_option('-V', '--verbose', dest='verbose',    action='store_true', default=False,
	help='Verbose mode.')
Parser.add_option('-D', '--debug',   dest='debug',      action='store_true', default=False,
	help='Debug mode.')

(Options, Args) = Parser.parse_args()

if Options.input == '': Parser.error('Input image not specified.')

Images = []
MTime = 0
if os.path.isfile( Options.output):
	MTime = os.path.getmtime( Options.output)
	if Options.verbose:
		print('Thumbnail "%s" already exists.' % Options.output)

if Options.input.find(',') != -1 or os.path.isdir( Options.input):
	folders = [Options.input]
	if folders[0].find(',') != -1:
		folders = folders[0].split(',')
	img_mtime = 0
	for folder in folders:
		if Options.verbose: print('Scanning folder "%s"...' % folder)
		for root, dirs, files in os.walk( folder):
			if len( files) == 0: continue
			images = []
			for afile in files:
				split = re.split( r'\d\.', afile)
				if len(split) > 1 and split[-1] in Extesions:
					images.append( afile)
			if len( images) == 0: continue
			new_mtime = int( os.path.getmtime(root))
			if new_mtime > img_mtime:
				Images = []
				images.sort()
				for i in range( Options.number):
					num = int( len(images) * (i+1.0) / (Options.number+1.0) )
					Images.append( os.path.join( root, images[num]))
				img_mtime = new_mtime
else:
	if not os.path.isfile( Options.input): Parser.error('Can`t fide input file:\n' + str( Options.input))

if len( Images ) == 0:
	print('ERROR: Can`t find images in "%s"' % Options.input)
	sys.exit(1)

if Options.verbose:
	print('Images:');
	for img in Images:
		print( img)

if MTime >= img_mtime:
	if Options.verbose:
		print('Thumbnail is up to date (%d %d)' % (MTime, img_mtime))
	if Options.force:
		if Options.verbose:
			print('Forcing thumbnail creation')
	else:
		sys.exit(0)

OutDir = os.path.dirname( Options.output )
if OutDir != '':
	if not os.path.isdir( OutDir):
		os.makedirs( OutDir)
		if not os.path.isdir( OutDir):
			print('ERROR: Can`t create output folder "%s"' % OutDir)
			sys.exit(1)

Cmds = []
Thumbnails = []
cmd = 'convert'
cmd += ' -resize %dx%d' % (Options.xres, Options.yres)
cmd += ' "%s" "%s"'
for i in range( len( Images)):
	thumbnail = os.path.join( OutDir, 'thumbnail_%d.jpg' % i)
	Cmds.append( cmd % ( Images[i], thumbnail))
	Thumbnails.append( thumbnail)

cmd = 'montage'
cmd += ' -geometry +0+0'
for img in Thumbnails:
	cmd += ' "%s"' % img
cmd += ' -alpha Off -strip'
cmd += ' "%s"' % Options.output
Cmds.append( cmd)

if Options.verbose or Options.debug:
	for cmd in Cmds:
		print( cmd)

if Options.debug:
	print('Debug mode. Exiting...')
	sys.exit( 0)

for cmd in Cmds:
	os.system( cmd)

