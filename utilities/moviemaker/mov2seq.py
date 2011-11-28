#!/usr/bin/env python

import os, sys
import re
import subprocess

if len(sys.argv) < 2:
   print('ERROR: Movie file not specicfied.')
   sys.exit(1)

inputmov = sys.argv[1]

if not os.path.isfile( inputmov):
   print('ERROR: Input movie file does not exist:')
   print( inputmov)
   sys.exit(1)

if len(sys.argv) > 2:
   outseq = sys.argv[2]
else:
   outseq = os.path.join( inputmov + '-png', 'frame.%07d.png')

outdir = os.path.dirname( outseq)
if not os.path.isdir( outdir): os.makedirs( outdir)

process = subprocess.Popen(['ffmpeg','-y','-i',inputmov,'-an','-f','image2',outseq], shell=False, stderr=subprocess.PIPE)
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
         if sys.version_info[0] < 3:
            print('Frame = %d' % frame),
            if frames_total != -1: print(' of %d' % frames_total),
            print(' ')
         else:
            print('Frame = %d' % frame, end = '')
            if frames_total != -1: print(' of %d' % frames_total, end = '')
            print()
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
