#!/usr/bin/env python

import os
import sys
import re
import time
import shutil
import signal
import subprocess

tmpdir = ''
def rmdir( signum, frame):
   print '\nInterrupt received...'
   if not debug:
      if os.path.isdir( tmpdir):
         shutil.rmtree( tmpdir)
         if os.path.isdir( tmpdir):
            print 'Warning: Temporary directory still exsists:'
            print tmpdir
      else:
         print 'Warning: Temporary directory does not exsist:'
         print tmpdir
   exit(0)

signal.signal(signal.SIGTERM, rmdir)
signal.signal(signal.SIGABRT, rmdir)
signal.signal(signal.SIGINT,  rmdir)

from optparse import OptionParser
parser = OptionParser(usage="%prog [options]\ntype \"%prog -h\" for help", version="%prog 1.0")

parser.add_option('-c', '--codec',      dest='codec',       type  ='string',     default='aphotojpg.ffmpeg',  help='File with encode command line in last line')
parser.add_option('-f', '--fps',        dest='fps',         type  ='int',        default=25,          help='Frames per second')
parser.add_option('-i', '--inpattern',  dest='inpattern',   type  ='string',     default='',          help='Input files pattern: img.####.jpg')
parser.add_option('-o', '--output',     dest='output',      type  ='string',     default='',          help='Output filename, if not specified, pattern will be used')
parser.add_option('-t', '--template',   dest='template',    type  ='string',     default='',          help='Specify frame template to use')
parser.add_option('-s', '--slate',      dest='slate',       type  ='string',     default='',          help='Specify slate frame template')
parser.add_option('--addtime',          dest='addtime',     action='store_true', default=False,       help='Draw time with date')
parser.add_option('--datesuffix',       dest='datesuffix',  action='store_true', default=False,       help='Add date suffix to output file name')
parser.add_option('--timesuffix',       dest='timesuffix',  action='store_true', default=False,       help='Add time suffix to output file name')
parser.add_option('-V', '--verbose',    dest='verbose',     action='store_true', default=False,       help='Verbose mode')
parser.add_option('-D', '--debug',      dest='debug',       action='store_true', default=False,       help='Debug mode (verbose mode, no commands execution)')
parser.add_option('-A', '--afanasy',    dest='afanasy',     action='store_true', default=False,       help='Send to Afanasy')
parser.add_option('--afconvcap',        dest='afconvcap',   type  ='int',        default=-1,          help='Afanasy convert tasks capacity, -1=default')
parser.add_option('--afenccap',         dest='afenccap',    type  ='int',        default=-1,          help='Afanasy encode task capacity, -1=default')
parser.add_option('--afuser',           dest='afuser',      type  ='string',     default='',          help='Change Afanasy job user, ''=current')
parser.add_option('--tmpdir',           dest='tmpdir',      type  ='string',     default='',          help='Temporary directory, if not specified, .makemovie+date will be used')
parser.add_option('--tmpformat',        dest='tmpformat',   type  ='string',     default='tga',       help='Temporary images format')

# Options to makeframe:
parser.add_option('-r', '--resolution', dest='resolution',  type  ='string',     default='',          help='Format: 768x576, if empty images format used')
parser.add_option('-g', '--gamma',      dest='gamma',       type  ='float',      default=-1.0,        help='Apply gamma correction')
parser.add_option('-q', '--quality',    dest='quality',     type  ='string',     default='',          help='Temporary image quality, or format options')
parser.add_option('--company',          dest='company',     type  ='string',     default='',          help='Draw company')
parser.add_option('--project',          dest='project',     type  ='string',     default='',          help='Draw project')
parser.add_option('--shot',             dest='shot',        type  ='string',     default='',          help='Draw shot')
parser.add_option('--ver',              dest='shotversion', type  ='string',     default='',          help='Draw shot version')
parser.add_option('--artist',           dest='artist',      type  ='string',     default='',          help='Draw artist')
parser.add_option('--activity',         dest='activity',    type  ='string',     default='',          help='Draw activity')
parser.add_option('--comments',         dest='comments',    type  ='string',     default='',          help='Draw comments')
parser.add_option('--font',             dest='font',        type  ='string',     default='',          help='Specify font)')
parser.add_option('--logopath',         dest='logopath',    type  ='string',     default='',          help='Add a specified image')
parser.add_option('--logosize',         dest='logosize',    type  ='string',     default='200x100',   help='Logotype size')
parser.add_option('--logograv',         dest='logograv',    type  ='string',     default='southeast', help='Logotype positioning gravity')
parser.add_option('--draw169',          dest='draw169',     type  ='int',        default=0,           help='Draw 16:9 cacher opacity')
parser.add_option('--draw235',          dest='draw235',     type  ='int',        default=0,           help='Draw 2.35 cacher opacity')

(options, args) = parser.parse_args()

codec       = options.codec
resolution  = options.resolution
inpattern   = options.inpattern
output      = options.output
logopath    = options.logopath
logosize    = options.logosize
datesuffix  = options.datesuffix
timesuffix  = options.timesuffix

verbose     = options.verbose
debug       = options.debug

tmpdir      = options.tmpdir
tmpformat   = options.tmpformat

# Parameters initialization:
if debug: verbose = True
if verbose: print 'VERBOSE MODE:'
if debug: print 'DEBUG MODE:'


# Definitions:
tmpname   = 'img'
tmplogo   = 'logo.png'

need_convert = False
need_logo = False

# Process encode command:
encoder = codec.split('.')
if len(encoder) < 1:
   print 'Invalid encode file "%s"' % codec
   exit(1)
encoder = encoder[len(encoder)-1]
if verbose: print 'Encoder engine = "%s"' % encoder
file = open( codec)
lines = file.readlines()
cmd_encode = lines[len(lines)-1].strip()
if len(cmd_encode) < 2:
   print 'Invalid encode file "%s"' % codec
   exit(1)
if verbose: print 'Encode command = "%s"' % cmd_encode

#datetimestring = '`date +%y-%m-%d_%H-%M`'
datetimestring = time.strftime('%y-%m-%d')
datetimesuffix = ''
if datesuffix: datetimesuffix += time.strftime('%y%m%d')
if options.addtime:
   if datetimestring != '': datetimestring += ' '
   datetimestring += time.strftime('%H:%M')
if timesuffix:
   if datetimesuffix != '': datetimesuffix += '_'
   datetimesuffix += time.strftime('%H%M')

# Check required parameters:
if inpattern == '': parser.error('Input files not specified.')

# Input directory:
inputdir = os.path.dirname( inpattern)
if verbose: print 'InputDir = "%s"' % inputdir
if not os.path.isdir( inputdir):
   print 'Can\'t find input directory "%s"' % inputdir
   exit(1)

# Input files pattern processing:
pattern = os.path.basename( inpattern)
digitspos = pattern.rfind('#')
if digitspos < 0:
   print 'Can\'t find # in input files pattern'
   exit(1)
digitsnum = 1
for i in range(digitspos):
   if pattern[digitspos-digitsnum] == '#':
      digitsnum += 1
   else:
      break
prefix = pattern[ 0 : digitspos-digitsnum+1 ]
suffix = pattern[ digitspos+1: len(pattern)]

# Input files search pattern:
eprefix = re.escape( prefix)
esuffix = re.escape( suffix)
expr = r'%(eprefix)s([0-9]{%(digitsnum)s,%(digitsnum)s})%(esuffix)s' % vars()
if verbose: print 'Expression = ' + expr
expr = re.compile( expr)
allFiles = []
allItems = os.listdir( inputdir)
for item in allItems:
   if not os.path.isfile( os.path.join( inputdir, item)): continue
   if not expr.match( item): continue
   allFiles.append(item)
if len(allFiles) == 0:
   print 'No files founded matching pattern.'
   print 'Input directory:'
   print inputdir
   print 'Expression:'
   print expr.pattern
   exit(1)
if len(allFiles) == 1:
   print 'Founded only 1 file matching pattern.'
   print 'Input directory:'
   print inputdir
   print 'Expression:'
   print expr.pattern
   exit(1)
allFiles.sort()
#print allFiles
if verbose: print 'Files fonded: %d' % len(allFiles)

# Input files indentify:
afile = os.path.join( inputdir, allFiles[0])
pipe = subprocess.Popen( 'identify "%s"' % afile, shell=True, bufsize=100000, stdout=subprocess.PIPE).stdout
identify = pipe.read()
identify = identify.replace( afile, '')
identify = identify.strip()
if len(identify) < 1:
   print 'Invalid image "%s"' % afile
   exit(1)
if verbose: print 'Identify: %s' % output
identify = identify.split(' ')
if len(identify) < 1:
   print 'Invalid image "%s"' % afile
   exit(1)
imgtype = identify[0]
if verbose: print 'Images type = "%s"' % imgtype

# Output file:
output = output.strip('" ')
if output == '': output = os.path.join( os.path.dirname( inputdir), prefix.strip('_. '))
afjobname = os.path.basename( output)
if datetimesuffix != '': output += '_' + datetimesuffix
if codec.find('xvid') != -1: output += '.avi'
else: output += '.mov'
if debug: output = os.path.basename( output)
if verbose: print 'Output = ' + output

# Temporary directory:
if not debug:
   if tmpdir == '':
      ftime = time.time()
      tmpdir = 'makemovie.' + time.strftime('%y-%m-%d_%H-%M-%S_') + str(ftime - int(ftime))[2:]
      if options.afanasy:
         tmpdir = os.path.join( os.path.dirname(output), '.' + tmpdir)
      else:
         tmp = os.getenv('TMPDIR', os.getenv('TMP', os.getenv('TEMP')))
         if tmp is None:
            if sys.platform.find('win') == 0: tmp = 'c:\\temp'
            else: tmp = '/tmp'
         tmpdir = os.path.join( tmp, tmpdir)
   if os.path.isdir( tmpdir): shutil.rmtree( tmpdir)
   print 'Temporary Directory:'
   print tmpdir

# Resolution:
width = 0
height = 0
if resolution != '':
   need_convert = True
   pos = resolution.find('x')
   if pos <= 0:
      print 'Invalid resolution specified.'
      exit(1)
   width = int(resolution[ 0 : pos ])
   height = int(resolution[ pos + 1 : len(resolution) ])
   if verbose: print 'Output resolution = %(width)d x %(height)d' % vars()
   afjobname += ' %s' % resolution

cmd_makeframe = os.path.join( os.path.dirname(sys.argv[0]), 'makeframe.py')
cmd_makeframe = 'python ' + cmd_makeframe

cmd_args = ''
if options.resolution   != '': cmd_args += ' -r %s' % options.resolution
if options.quality      != '': cmd_args += ' -q %s' % options.quality
if options.company      != '': cmd_args += ' -c "%s"' % options.company
if options.project      != '': cmd_args += ' -p "%s"' % options.project
if options.artist       != '': cmd_args += ' -a "%s"' % options.artist
if options.shot         != '': cmd_args += ' -s "%s"' % options.shot
if options.shotversion  != '': cmd_args += ' --ver "%s"'       % options.shotversion
if options.font         != '': cmd_args += ' --font "%s"'      % options.font
if options.activity     != '': cmd_args += ' --activity "%s"'  % options.activity
if options.comments     != '': cmd_args += ' --comments "%s"'  % options.comments
cmd_args += ' -d "%s"' % datetimestring
cmd_args += ' -m "%s"' % os.path.basename(output)


imgCount = 0
# Pre composition:
cmd_precomp = []
name_precomp = []
# Generate header:
if need_convert and options.slate != '':
   cmd = cmd_makeframe + cmd_args
   cmd += ' --drawcolorbars' + cmd_args
   cmd += ' -t "%s"' % options.slate
   cmd += ' "%s"' % (os.path.join( tmpdir, tmpname) + '.%07d.' % imgCount + tmpformat)
   cmd_precomp.append(cmd)
   name_precomp.append('Generate header')
   imgCount += 1

# Reformat logo command:
if logopath != '':
   if need_convert:
      need_logo = True
      logosizes = logosize.split('x')
      logotx = width  - int(logosizes[0])
      logoty = height - int(logosizes[1])
      tmplogo = os.path.join( tmpdir, tmplogo)
      cmd = 'convert'
      cmd += ' "%s"' % logopath
      cmd += ' -gravity %s -background black' % options.logograv
      cmd += ' -resize ' + logosize
      cmd += ' -extent %(width)dx%(height)d' % vars()
      cmd += ' "%s"' % tmplogo
   else:
      print 'Can\'t add logo if output resolution is not specified.'
      exit(1)
   cmd_precomp.append(cmd)
   name_precomp.append('Reformat logo')

# Generate convert commands lists:
cmd_convert = []
name_convert = []
if need_convert:
   for afile in allFiles:
      cmd = cmd_makeframe + cmd_args
      if options.template != '': cmd += ' -t "%s"' % options.template
      if options.gamma     >  0: cmd += ' -g %.2f'     % options.gamma
      if options.draw169   >  0: cmd += ' --draw169 %d' % options.draw169
      if options.draw235   >  0: cmd += ' --draw235 %d' % options.draw235
      if need_logo:              cmd += ' --logopath "%s"' % tmplogo

      cmd += ' "%s"' % os.path.join( inputdir, afile)
      cmd += ' "%s"' % (os.path.join( tmpdir, tmpname) + '.%07d.' % imgCount + tmpformat)

      cmd_convert.append( cmd)
      name_convert.append( afile)
      imgCount += 1

# Encode commands:
auxargs = ''
if codec != 'ffmpeg':
   if not sys.platform.find('win') == 0: auxargs = '-threads 0'
   inputmask = os.path.join( inputdir, prefix+'%0'+str(digitsnum)+'d'+suffix)
   if len(cmd_convert): inputmask = os.path.join( tmpdir, tmpname+'.%07d.'+tmpformat)
elif codec == 'mencoder':
   inputmask = os.path.join( inputdir, prefix+'*'+suffix)
   if len(cmd_convert): inputmask = os.path.join( tmpdir, tmpname+'.*.'+tmpformat)
else:
   print 'Unknown encoder = "%s"' % encoder
   exit(1)
cmd_encode = cmd_encode.replace('@MOVIEMAKER@', os.path.dirname(sys.argv[0]))
cmd_encode = cmd_encode.replace('@INPUT@',      inputmask)
cmd_encode = cmd_encode.replace('@FPS@',        str(options.fps))
cmd_encode = cmd_encode.replace('@OUTPUT@',     output)
cmd_encode = cmd_encode.replace('@ARGS@',       auxargs)

# Print commands:
if debug:
   if len(cmd_precomp):
      print 'Precomp  first and last commands:'
      print
      print cmd_precomp[0]
      os.system( cmd_precomp[0])
      if len(cmd_precomp) > 1:
         print '...'
         print cmd_precomp[-1]
         os.system( cmd_precomp[-1])
      print
   if need_convert:
      print 'Convert first and last commands:'
      print
      print cmd_convert[0]
      os.system( cmd_convert[0])
      print '...'
      print cmd_convert[-1]
      os.system( cmd_convert[-1])
      print
   print 'Encode command:'
   print
   print cmd_encode
   os.system( cmd_encode)
   print
   exit(0)

# Construct Afanasy job:
if options.afanasy:
   af = __import__('af', globals(), locals(), [])
   j=af.Job( afjobname)
   if options.afuser != '': j.setUserName( options.afuser)

   if len(cmd_precomp):
      bp=af.Block( 'precomp', 'generic')
      j.blocks.append( bp)
      n = 0
      for cmd in cmd_precomp:
         t=af.Task( name_precomp[n])
         bp.tasks.append( t)
         t.setCommand( cmd)
         n += 1
      bp.setCapacity( options.afconvcap)

   if need_convert:
      bc=af.Block( 'convert', 'generic')
      j.blocks.append( bc)
      n = 0
      for cmd in cmd_convert:
         t=af.Task( name_convert[n])
         bc.tasks.append( t)
         t.setCommand( cmd)
         n += 1
      bc.setCapacity( options.afconvcap)
      if need_logo: bc.setDependMask('precomp')
      bc.setTasksMaxRunTime(11)

   be = af.Block( 'encode', 'generic')
   j.blocks.append( be)
   t = af.Task( output)
   be.tasks.append( t)
   t.setCommand( cmd_encode)
   be.setCapacity( options.afenccap)
   if need_convert:
      be.setDependMask('convert')
      j.setCmdPre( 'mkdir "%s"' % os.path.abspath(tmpdir))
      j.setCmdPost('rm -rf "%s"' % os.path.abspath(tmpdir))

   if verbose: j.output(1)

# Commands execution:
if options.afanasy: j.send( verbose)
else:
   if len(cmd_precomp) or need_convert: os.mkdir(tmpdir, 0777)
   if len(cmd_precomp):
      n = 0
      print 'Precomositing...'
      for cmd in cmd_precomp:
         print name_precomp[n]
         os.system( cmd)
         n += 1
      print
   if need_convert:
      n = 0
      print 'Converting...',
      for cmd in cmd_convert:
         print name_convert[n]
         os.system( cmd)
         n += 1
         print 'PROGRESS: %d' % (100.0 * n / imgCount) + '%'
         sys.stdout.flush()
      print
   os.system( cmd_encode)
   if not debug:
      if os.path.isdir( tmpdir):
         shutil.rmtree( tmpdir)
         if os.path.isdir( tmpdir):
            print 'Warning: Temporary directory still exsists:'
            print tmpdir
      else:
         print 'Warning: Temporary directory does not exsist:'
         print tmpdir
   print
   print 'Done'
