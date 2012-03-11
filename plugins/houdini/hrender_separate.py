#!/usr/bin/env python

import subprocess, os, sys, signal, tempfile, shutil
from optparse import OptionParser

tmpdir = ''
# Interrupt function to delete temp directory:
def interrupt( signum, frame):
   if tmpdir != '':
      if os.path.isdir( tmpdir):
         shutil.rmtree( tmpdir)
   exit('\nInterrupt received (hrender_separate.py)...')

# Set interrupt function:
signal.signal( signal.SIGTERM, interrupt)
signal.signal( signal.SIGABRT, interrupt)
if sys.platform.find('win') != 0: signal.signal( signal.SIGQUIT, interrupt)
signal.signal( signal.SIGINT,  interrupt)

parser = OptionParser( usage="usage: %prog [options] hip_name rop_name", version="%prog 1.0")
parser.add_option('-s', '--start',     dest='start',     type='float',  help='Start frame number.')
parser.add_option('-e', '--end',       dest='end',       type='float',  help='End frame number.')
parser.add_option('-b', '--by',        dest='by',        type='float',  help='Frame increment.')
parser.add_option('-t', '--take',      dest='take',      type='string', help='Take name.')
parser.add_option('-o', '--out',       dest='output',    type='string', help='Output file.')
parser.add_option(      '--numcpus',   dest='numcpus',   type='int',    help='Number of CPUs.')
parser.add_option('-i', '--ignore_inputs', action='store_true', dest='ignore_inputs', default=False, help='Ignore inputs')
parser.add_option(      '--tmpimg',        action='store_true', dest='tmpimg',        default=False, help='Use local temporary images folder.')

(options, args) = parser.parse_args()

start    = options.start
end      = options.end
by       = options.by
take     = options.take
numcpus  = options.numcpus
output   = options.output
tmpimg   = options.tmpimg
ignoreInputs = options.ignore_inputs

if len(args) < 2:
   parser.error( 'At least one of mandatory rop_name or hip_name argument is missed.')
elif len(args) > 2:
   parser.error( 'Too many arguments provided.')
else:
   hip = args[0]
   rop = args[1]

# If end wasn't specified, render single frame
if end is None:
   end = start

# If by wasn't specified, render every frame
if by is None:
   by = 1

cmd = os.getenv('HOUDINI_CGRU_PATH')
if cmd is None:
   print('Error: HOUDINI_CGRU_PATH is not set.')
   exit(1)

tmpdir = tempfile.mkdtemp('.afrender.houdini')
if os.path.exists( tmpdir):
   print('Generating files in temporary directory:')
   print(tmpdir)
else:
   print('Error creating temp directory.')
   sys.exit(1)

hython = 'hython'
app_dir = os.getenv('APP_DIR')
if app_dir is not None:
   app_dir = os.path.join( app_dir, 'bin')
   hython = os.path.join( app_dir, 'hython')
   if sys.platform.find('win') == 0: hython += '.exe'

cmdgen_s = [ hython, os.path.join( cmd,'hrender_af.py'),'-s']
cmdgen_e = []
if ignoreInputs: cmdgen_e.append('-i')
if take    is not None: cmdgen_e.extend(['-t', take ])
if output  is not None: cmdgen_e.extend(['--output',  str(output) ])
if numcpus is not None: cmdgen_e.extend(['--numcpus', str(numcpus)])
cmdgen_e.append(hip)
cmdgen_e.append(rop)

cmdrnd = ['python', os.path.join( cmd, 'mantrarender.py')]
if tmpimg: cmdrnd.append('dt')
else:      cmdrnd.append('d')
cmdrnd.extend(['-R','-f'])

frame = start
while frame <= end:
   # Construct ROP command:
   cmd = []
   cmd.extend( cmdgen_s)
   cmd.append(str(frame))
   cmd.append('--diskfile')
   afile = os.path.join( tmpdir, 'scene.' + str(frame) + '.ifd')
   cmd.append( afile )
   cmd.extend( cmdgen_e)

   # Run ROP command
   print('Launching command:')
   for c in cmd: print(c,)
   print('\n')
   sys.stdout.flush()
   p = subprocess.Popen( cmd)
   exitcode = p.wait()
   if exitcode != 0: break

   # Construct render command:
   cmd = []
   cmd.extend( cmdrnd)
   cmd.append( afile)

   # Launch render command:
   print('Launching command:')
   for c in cmd: print(c,)
   print('\n')
   sys.stdout.flush()
   p = subprocess.Popen( cmd)
   exitcode = p.wait()
   if exitcode != 0: break

   frame += by

if tmpdir != '':
   # Remove temp directory:
   try:      
      shutil.rmtree( tmpdir)
   except:
      print('Unable to remove temporary directory:')
      print(tmpdir)
      print(str(sys.exc_info()[1]))

sys.exit(exitcode)
