#!/usr/bin/env python

import os, sys
import re
import subprocess

from optparse import OptionParser
Parser = OptionParser(usage="%prog [options] input [second_input] output\ntype \"%prog -h\" for help", version="%prog 1.  0")

Parser.add_option('-a', '--avconv',    dest='avconv',    type  ='string', default='ffmpeg', help='AV convert command (ffmpeg)')
Parser.add_option('-x', '--xres',      dest='xres',      type  ='int',    default=-1,       help='X resolution (no change)')
Parser.add_option('-y', '--yres',      dest='yres',      type  ='int',    default=-1,       help='Y resolution (no change)')
Parser.add_option('-c', '--codec',     dest='codec',     type  ='string', default='',       help='Movie cpdec (png)')
Parser.add_option('-f', '--fps'  ,     dest='fps',       type  ='string', default='24',     help='Movie FPS (24)')
Parser.add_option('-n', '--container', dest='container', type  ='string', default='mov',    help='Movie Container (mov or ogg for theora)')
Parser.add_option('-t', '--type',      dest='type',      type  ='string', default='png',    help='Images type (png)')
Parser.add_option('-o', '--output',    dest='output',    type  ='string', default='',       help='Output movie or images folder (auto)')
Parser.add_option('-q', '--qscale',    dest='qscale',    type  ='int',    default=5,        help='JPEG compression rate (5)')
Parser.add_option(      '--imgname',   dest='imgname',   type  ='string', default='frame',  help='Images files name (frame)')

(Options, argv) = Parser.parse_args()

if len(argv) < 1:
	print('ERROR: Movie file not specicfied.')
	sys.exit(0)

Input = argv[0]

if not os.path.isfile( Input):
	print('ERROR: Input movie file does not exist:'+Input)
	sys.exit(1)

MOVIEMAKER = os.path.dirname( sys.argv[0])
CODECSDIR  = os.path.join( MOVIEMAKER, 'codecs')

Output = Options.output
Codec = Options.codec

if Codec == '':
	args = [ Options.avconv,'-y','-i', Input ]
	args.extend(['-an','-f','image2'])
	if Options.type == 'jpg':
		args.extend(['-qscale', str(Options.qscale)])
		Output += '.q'+str(Options.qscale)
	if not os.path.isdir( Output): os.makedirs( Output)
	if not os.path.isdir( Output):
		print('ERROR: Can`t create output folder: '+Output)
		sys.exit(1)

	if Options.xres != -1 or Options.yres != -1:
		args.extend(['-vf','scale=%d:%d' % (Options.xres,Options.yres)])
		if Options.xres != -1: Output += '.'+str(Options.xres)
		if Options.yres != -1: Output += 'x'+str(Options.yres)

	if Output == '':
		Output = os.path.join( Input +'.'+Options.type)
	Output = os.path.join( Output, Options.imgname+'.%07d.'+Options.type)

	args.append( Output)

else:
	args = []
	if Codec.find('.') == -1: Codec += '.ffmpeg'
	if os.path.dirname( Codec) == '': Codec = os.path.join( CODECSDIR, Codec)
	if not os.path.isfile( Codec):
		print('ERROR: Can`t find codec "%s"' % Codec)
		sys.exit(1)
	file = open( Codec)
	lines = file.readlines()
	cmd_enc = lines[len(lines)-1].strip()
	if len( cmd_enc) < 2:
		print('Invalid encode file "%s"' % Codec)
		sys.exit(1)

	if Output != '': Output += '/'
	Output += Input + '.' + os.path.basename( Codec.split('.')[0])

	auxargs = []
	if Options.xres != -1 or Options.yres != -1:
		auxargs = ['-vf','scale=%d:%d' % (Options.xres,Options.yres)]
		if Options.xres != -1: Output += '.'+str(Options.xres)
		if Options.yres != -1: Output += 'x'+str(Options.yres)

	cmd_enc = cmd_enc.split(' ')
	for arg_enc in cmd_enc:
		if arg_enc[0]  == '"': arg_enc = arg_enc[1:]
		if arg_enc[-1] == '"': arg_enc = arg_enc[:-1]

		arg_enc = arg_enc.replace('@MOVIEMAKER@', MOVIEMAKER        )
		arg_enc = arg_enc.replace('@CODECS@',     CODECSDIR         )
		arg_enc = arg_enc.replace('@INPUT@',      Input             )
		arg_enc = arg_enc.replace('@FPS@',        Options.fps       )
		arg_enc = arg_enc.replace('@CONTAINER@',  Options.container )
		arg_enc = arg_enc.replace('@OUTPUT@',     Output            )

		if arg_enc == '@AUXARGS@':
			args.extend( auxargs)
			continue

		if len( arg_enc):
			args.append( arg_enc)

print( args)

try:
	process = subprocess.Popen( args, shell=False, stderr=subprocess.PIPE)
except:
	print('Command execution error:')
	print( str(sys.exc_info()[1]))
	sys.exit(1)

re_duration = re.compile(r'Duration: (\d\d:\d\d:\d\d)\.(\d\d)')
re_fps = re.compile(r'Stream.*: Video:.*(\d\d) fps')

seconds = -1
frames_total = -1
fps = -1
frame = -1
progress = -1
frame_old = -1
framereached = False
output = ''
while True:
	stdout = ''
	data = process.stderr.read(1)
	if data is None: break
	if len(data) < 1: break
	if not isinstance( data, str): data = str( data, 'ascii')
	data = data.replace('\r','\n')
	sys.stdout.write( data)
	if data == '\n':
		output = ''
		if frame_old != frame:
			frame_info = 'Frame = %d' % frame
			if frames_total != -1: frame_info += ' of %d' % frames_total
			print( frame_info)
			if progress != -1: print('PROGRESS: %d%%' % progress)
			frame_old = frame
		sys.stdout.flush()
		continue
	output += str(data)

	if seconds == -1 and frame == -1:
		reobj = re_duration.search( output)
		if reobj is not None:
			time_s, time_f = reobj.groups()
			time_s = time_s.split(':')
			time_slen = len(time_s)
			if time_slen > 0:
				seconds = 0
				i = time_slen - 1
				mult = 1
				while i >= 0:
					seconds += int(time_s[i]) * mult
					mult *= 60
					i -= 1
				seconds = seconds * 100 + int(time_f)
			output = ''
			continue

	if fps == -1 and frame == -1:
		reobj = re_fps.search( output)
		if reobj is not None:
			fps = int(reobj.groups()[0])
			output = ''
			continue

	if frame == -1 and fps != -1 and seconds != -1 and frames_total == -1:
		frames_total = 1 + seconds * fps / 100
		progress = 0

	if output == 'frame=':
		framereached = True
		output = ''
		continue
	if framereached and output[-4:] == 'fps=':
		try:
			frame = int(output[:-4])
			if progress != -1 and frames_total > 0: progress = 100 * frame / frames_total
		except:
			print(str(sys.exc_info()[1]))
		framereached = False
		output = ''
	continue
