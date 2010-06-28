#!/usr/bin/env python

import subprocess, os, sys, signal, tempfile, shutil

# Interrupt function to delete temp directory:
def interrupt( signum, frame):
   if tmpdir != '':
      if os.path.isdir( tmpdir):
         shutil.rmtree( tmpdir)
   exit('\nInterrupt received...')

# Set interrupt function:
signal.signal( signal.SIGTERM, interrupt)
signal.signal( signal.SIGABRT, interrupt)
if sys.platform.find('win') != 0: signal.signal( signal.SIGQUIT, interrupt)
signal.signal( signal.SIGINT,  interrupt)

# Check environment:
filter = os.getenv('HOUDINI_CGRU_PATH')
if filter is None or filter == '':
   print 'Error: HOUDINI_CGRU_PATH is not set, can`t find mantra pyhton filer location.'
   exit(1)

def UsageExit( msg = None):
   if msg is not None: print msg
   else: print 'Error: Invalid arguments.'
   print 'Usage: ' + os.path.basename(sys.argv[0]) + ' [tc] [divx divy numtile] -R [argumets to mantra]'
   print 't - enable render in temporary folder'
   print 'c - crop render region to render one tile'
   print 'divx, divy, numtile - tile render options'
   print '-R - seperator for arguments to mantra ("-v A" will be added automatically)'
   exit(1)

tmpdir = ''
tilerender = False

# Mantra arguments position:
argspos = 0
if '-R' not in sys.argv: UsageExit()
for arg in sys.argv:
   if arg != '-R': argspos += 1
   else: break
if argspos >= ( len(sys.argv) - 1): UsageExit('No arguments for render command specified.')
if argspos == 1: UsageExit('No arguments for filter specified.')

# Tile render:
if 'c' in sys.argv[1]:
   divx=int( sys.argv[2])
   divy=int( sys.argv[3])
   numtile=int( sys.argv[4])
   if divx < 1: divx = 1
   if divy < 1: divy = 1
   if numtile < 0: numtile = 0
   if numtile >= divx * divy: numtile = divx * divy - 1
   tilerender = True

# Temp directory:
if 't' in sys.argv[1]:
   tmpdir = tempfile.mkdtemp('.afrender.mantra')
   if os.path.exists( tmpdir):
      print 'Rendering in temporary directory:'
      print tmpdir
   else:
      print 'Error creating temp directory.'
      sys.exit(1)

filter = filter.replace('\\','/')
filter += '/mantrafilter.py'
if tilerender: filter += ' %d %d %d' % ( divx, divy, numtile)
if tmpdir != '': filter += ' ' + tmpdir

if sys.platform.find('win') == 0:
   houdini = os.getenv('HOUDINI_LOCATION')
   if houdini is None or houdini == '':
      print 'Error: HOUDINI_LOCATION is not set, can`t find mantra location.'
      exit(1)
   mantra = houdini + '/bin/mantra.exe'
else:
   mantra = 'mantra'

# Construcvt command:
cmd = [ mantra,'-P',filter,'-v','A']
for i in range( argspos+1, len(sys.argv)): cmd.append( sys.argv[i])

# Launch mantra:
p = subprocess.Popen( cmd)
exitcode = p.wait()

if tmpdir != '':
   # Remove temp directory:
   try:      
      shutil.rmtree( tmpdir)
   except:
      print 'Unable to remove temporary directory:'
      print tmpdir
      print str(sys.exc_info()[1])

sys.exit(exitcode)
