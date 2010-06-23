#!/usr/bin/env python

import subprocess, os, sys, signal

if len(sys.argv) < 4:
   print 'Error: Invalid arguments.'
   print 'Usage: ' + os.path.basename(sys.argv[0]) + ' divx divy tile argumets to mantra'
   exit(1)

divx=int( sys.argv[1])
divy=int( sys.argv[2])
tile=int( sys.argv[3])

filter = os.getenv('HOUDINI_CGRU_PATH')
if filter is None or filter == '':
   print 'Error: HOUDINI_CGRU_PATH is not set, can`t find mantra pyhton filer location.'
   exit(1)

filter = filter.replace('\\','/')

filter += '/mantracropfilter.py'
filter += ' %d %d %d' % ( divx, divy, tile)
cmd = ['mantra','-P',filter]
for i in range( 4, len(sys.argv)): cmd.append( sys.argv[i])

p = subprocess.Popen( cmd)
sts = p.wait()
exit(sts)
