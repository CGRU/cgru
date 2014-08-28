#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import sys
import re
import subprocess

from optparse import OptionParser

Parser = OptionParser(
	usage="%prog [options] input\ntype \"%prog -h\" for help",
	version="%prog 1.  0"
)

Parser.add_option('-a', '--avcmd',     dest='avcmd',     type  ='string', default='ffmpeg', help='AV convert command')
Parser.add_option('-r', '--resize',    dest='resize',    type  ='string', default='',       help='Resize (1280x720)')
Parser.add_option('-c', '--codec',     dest='codec',     type  ='string', default='',       help='Movie codec')
Parser.add_option('-f', '--fps'  ,     dest='fps',       type  ='string', default='24',     help='Movie FPS (24)')
Parser.add_option('-n', '--container', dest='container', type  ='string', default='mov',    help='Movie Container (mov or ogg for theora)')
Parser.add_option('-t', '--type',      dest='type',      type  ='string', default='png',    help='Images type (png)')
Parser.add_option('-o', '--output',    dest='output',    type  ='string', default='',       help='Output movie or images folder (auto)')
Parser.add_option('-q', '--qscale',    dest='qscale',    type  ='int',    default=5,        help='JPEG compression rate (5)')
Parser.add_option('-s', '--timestart', dest='timestart', type  ='string', default='',       help='Time start')
Parser.add_option('-d', '--duration',  dest='duration',  type  ='string', default='',       help='Duration')
Parser.add_option(      '--imgname',   dest='imgname',   type  ='string', default=None,     help='Images files name (frame)')

Options, argv = Parser.parse_args()

if len(argv) < 1:
	print('ERROR: Movie file not specified.')
	sys.exit(0)

Input = argv[0]
Output = Options.output
if Output == '':
	Output = Input

SequenceInput = False
MovieInput = True
StartNumber = None
if os.path.isdir(Input):
	SequenceInput = True
	MovieInput = False
	allfiles = os.listdir(Input)
	allfiles.sort()
	for afile in allfiles:
		if afile[0] == '.':
			continue
		if not os.path.isfile(os.path.join(Input, afile)):
			continue
		digits = re.findall(r'\d+', afile)
		if len(digits) == 0:
			continue
		digits = digits[-1]
		StartNumber = int(digits)
		Input = afile[:afile.rfind(digits)]
		Input += '%0' + str(len(digits)) + 'd'
		Input += afile[afile.rfind(digits) + len(digits):]
		Input = os.path.join(argv[0], Input)
		break
else:
	if not os.path.isfile(Input):
		print('ERROR: Input does not exist: ' + Input)
		sys.exit(1)

MOVIEMAKER = os.path.dirname(sys.argv[0])
CODECSDIR = os.path.join(MOVIEMAKER, 'codecs')

Codec = Options.codec

if Codec == '':
	Output += '.' + Options.type
	args = [Options.avcmd, '-y']
	if Options.timestart != '':
		args.extend(['-ss', Options.timestart])
	args.extend(['-i', Input])
	if Options.duration != '':
		args.extend(['-t', Options.duration])
	args.extend(['-an', '-f', 'image2'])
	if Options.type == 'jpg':
		args.extend(['-qscale', str(Options.qscale)])
		Output += '.q' + str(Options.qscale)
	elif Options.type == 'dpx':
		args.extend(['-pix_fmt','gbrp10be'])
	elif Options.type == 'tif' or Options.type == 'tif8':
		args.extend(['-pix_fmt','rgb24'])
		Options.type = 'tif'
	elif Options.type == 'tif16':
		args.extend(['-pix_fmt','rgb48le'])
		Options.type = 'tif'

	if Options.resize != '':
		resize = Options.resize.split('x')
		if len(resize) < 2:
			resize.append('-1')
		args.extend(['-vf', 'scale=%s:%s' % (resize[0], resize[1])])
		Output += '.r%s' % Options.resize

	if not os.path.isdir(Output):
		os.makedirs(Output)
	if not os.path.isdir(Output):
		print('ERROR: Can`t create output folder: ' + Output)
		sys.exit(1)

	imgname = Options.imgname
	if imgname is None:
		imgname = os.path.basename( Input)
		imgname,ext = os.path.splitext( imgname)
	Output = os.path.join(Output, imgname + '.%07d.' + Options.type)

	args.append(Output)

else:
	args = []
	if Codec.find('.') == -1:
		Codec += '.ffmpeg'
	if os.path.dirname(Codec) == '':
		Codec = os.path.join(CODECSDIR, Codec)
	if not os.path.isfile(Codec):
		print('ERROR: Can`t find codec "%s"' % Codec)
		sys.exit(1)

	with open(Codec) as f:
		lines = f.readlines()

	cmd_enc = lines[len(lines) - 1].strip()
	if len(cmd_enc) < 2:
		print('Invalid encode file "%s"' % Codec)
		sys.exit(1)

	Output += '.' + os.path.basename(Codec.split('.')[0])

	auxargs = []
	if Options.timestart != '':
		auxargs.extend(['-ss', Options.timestart])
	if Options.duration != '':
		auxargs.extend(['-t', Options.duration])
	if Options.resize != '':
		resize = Options.resize.split('x')
		if len(resize) < 2:
			resize.append('-1')
		auxargs.extend(['-vf', 'scale=%s:%s' % (resize[0], resize[1])])
		Output += '.r%s' % Options.resize

	cmd_enc = cmd_enc.split(' ')
	for arg_enc in cmd_enc:
		if arg_enc[0] == '"':
			arg_enc = arg_enc[1:]
		if arg_enc[-1] == '"':
			arg_enc = arg_enc[:-1]

		arg_enc = arg_enc.replace('@MOVIEMAKER@', MOVIEMAKER)
		arg_enc = arg_enc.replace('@CODECSDIR@', CODECSDIR)
		arg_enc = arg_enc.replace('@INPUT@', Input)
		arg_enc = arg_enc.replace('@FPS@', Options.fps)
		arg_enc = arg_enc.replace('@CONTAINER@', Options.container)
		arg_enc = arg_enc.replace('@OUTPUT@', Output)

		if arg_enc == '@AVCMD@':
			args.append(Options.avcmd)
			if StartNumber:
				args.extend(['-start_number', str(StartNumber)])
		elif arg_enc == '@AUXARGS@':
			args.extend(auxargs)
		elif len(arg_enc):
			args.append(arg_enc)

print(' '.join(args))

try:
	process = subprocess.Popen(args, shell=False, stderr=subprocess.PIPE)
except Exception as e:
	print('Command execution error:')
	print(str(e))
	sys.exit(1)

re_duration = re.compile(r'Duration: (\d\d:\d\d:\d\d)\.(\d\d)')
re_fps = re.compile(r'Stream.*: Video:.*(\d\d) fps')

seconds      = -1
frames_total = -1
fps          = -1
frame        = -1
progress     = -1
frame_old    = -1
framereached = False
output       = ''
while True:
	data = process.stderr.read(1)
	if data is None:
		break
	#print( str( type( data)))
	if len(data) < 1:
		break
	if not isinstance(data, str):
		try:
			data = str(data, 'ascii')
		except:
			data = '@'
	data = data.replace('\r', '\n')
	sys.stdout.write(data)
	if data[-1] == '\n':
		output = ''
		if frame_old != frame:
			frame_info = 'Frame = %d' % frame
			if frames_total != -1:
				frame_info += ' of %d' % frames_total
			print(frame_info)
			if progress != -1:
				print('PROGRESS: %d%%' % progress)
			frame_old = frame
		sys.stdout.flush()
		continue
	output += data

	if seconds == -1 and frame == -1:
		reobj = re_duration.search(output)
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
		reobj = re_fps.search(output)
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
			if progress != -1 and frames_total > 0:
				progress = 100 * frame / frames_total
		except Exception as e:
			print(str(e))
		framereached = False
		output = ''
	continue
