#!/usr/bin/env python

import os, sys
import subprocess

if len(sys.argv) < 2:
   print('ERROR: Movie file not specicfied.')
   sys.exit(1)

inputmov = sys.argv[1]

if not os.path.isfile( inputmov):
   print('ERROR: Input movie file does not exist.')
   sys.exit(1)

if len(sys.argv) > 2:
   outseq = sys.argv[2]
else:
   outseq = os.path.join( inputmov + '-png', 'frame.%07d.png')

outdir = os.path.dirname( outseq)
if not os.path.isdir( outdir): os.makedirs( outdir)

cmd = 'ffmpeg'
cmd += ' -i "%s"' % inputmov
cmd += ' -an -f image2'
cmd += ' "%s"' % outseq

print( cmd)
os.system( cmd)
