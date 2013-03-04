#!/usr/bin/env python

import os, sys
import re
import subprocess

from optparse import OptionParser
Parser = OptionParser(usage="%prog [options] input [second_input] output\ntype \"%prog -h\" for help", version="%prog 1.  0")

Parser.add_option('-a', '--avconv', dest='avconv', type  ='string', default='ffmpeg', help='AV convert command (ffmpeg)')
Parser.add_option('-t', '--type',   dest='type',   type  ='string', default='png',    help='Images type (png)')
Parser.add_option('-n', '--name',   dest='name',   type  ='string', default='frame',  help='Images files name (frame)')
Parser.add_option('-o', '--outdir', dest='outdir', type  ='string', default='',       help='Output folder (auto)')
Parser.add_option('-x', '--xres',   dest='xres',   type  ='int',    default=-1,       help='Images x resolution (no change)')
Parser.add_option('-y', '--yres',   dest='yres',   type  ='int',    default=-1,       help='Images y resolution (no change)')
Parser.add_option('-q', '--qscale', dest='qscale', type  ='int',    default=5,        help='JPEG compression rate (5)')

(Options, argv) = Parser.parse_args()

if len(argv) < 1:
	print('ERROR: Movie file not specicfied.')
	sys.exit(0)

inputmov = argv[0]

if not os.path.isfile( inputmov):
	print('ERROR: Input movie file does not exist:'+inputmov)
	sys.exit(1)

outseq = Options.outdir
if outseq == '':
	outseq = os.path.join( inputmov +'.'+Options.type)

args = [ Options.avconv ]
args.extend(['-y','-i', inputmov,'-an','-f','image2'])
if Options.type == 'jpg':
	args.extend(['-qscale', str(Options.qscale)])
	outseq += '.q'+str(Options.qscale)
if Options.xres != -1 or Options.yres != -1:
	args.extend(['-vf','scale=%d:%d' % (Options.xres,Options.yres)])
	if Options.xres != -1: outseq += '.'+str(Options.xres)
	if Options.yres != -1: outseq += 'x'+str(Options.yres)

if not os.path.isdir( outseq): os.makedirs( outseq)
if not os.path.isdir( outseq):
	print('ERROR: Can`t create output folder: '+outseq)
	sys.exit(1)

outseq = os.path.join( outseq, Options.name+'.%07d.'+Options.type)

args.append( outseq)
#print( args)

try:
	process = subprocess.Popen( args, shell=False, stderr=subprocess.PIPE)
except:
	print('Command execution error:')
	print( str(sys.exc_info()[1]))
	sys.exit(1)
#cmd = 'ffmpeg -y -i "%s" -an -sn -f image2 "%s"' % ( inputmov, outseq)
#print(cmd)
#process = subprocess.Popen( cmd, shell=True, stderr=subprocess.PIPE)

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


