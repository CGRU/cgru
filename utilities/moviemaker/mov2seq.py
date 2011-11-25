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
#os.system( cmd)

#process = subprocess.Popen( cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
#process = subprocess.Popen( cmd+' 2>&1', shell=True, stdout=subprocess.PIPE)
#process = subprocess.Popen( cmd, shell=True, stderr=subprocess.PIPE)
#process = subprocess.Popen(['ffmpeg','-i',inputmov,outseq], shell=False, stderr=subprocess.PIPE)
process = subprocess.Popen( cmd+' | tr \\r \\n\\n', shell=True, stderr=subprocess.PIPE)
while True:
   stdout = ''
#   stdout = process.stdout.readline()
   stderr = process.stderr.readline()
#   stderr = process.stderr.read(10)
   if stdout is None and stderr is None: break
   if len(stdout) < 1 and len(stderr) < 1: break
   print('############## "%s"' % (stdout + stderr).replace('\r','\n'))
#print(process)
#for line in process:
#   print('\n############## %s\n' % line)

