#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import re
import time
import shutil
import signal
import subprocess

TmpDir = ''
def rmdir( signum, frame):
   print '\nInterrupt received...'
   if not Debug:
      if os.path.isdir( TmpDir):
         shutil.rmtree( TmpDir)
         if os.path.isdir( TmpDir):
            print 'Warning: Temporary directory still exsists:'
            print TmpDir
      else:
         print 'Warning: Temporary directory does not exsist:'
         print TmpDir
   exit(0)

signal.signal(signal.SIGTERM, rmdir)
signal.signal(signal.SIGABRT, rmdir)
signal.signal(signal.SIGINT,  rmdir)

from optparse import OptionParser
Parser = OptionParser(usage="%prog [Options] input_files_pattern(s)] output\n\
   Pattern examples = \"img.####.jpg\" or \"img.%04d.jpg\".\n\
   Type \"%prog -h\" for help", version="%prog 1.0")

Parser.add_option('-c', '--codec',      dest='codec',       type  ='string',     default='aphotojpg.ffmpeg',  help='File with encode command line in last line')
Parser.add_option('-f', '--fps',        dest='fps',         type  ='string',     default=25,          help='Frames per second')
Parser.add_option('-t', '--template',   dest='template',    type  ='string',     default='',          help='Specify frame template to use')
Parser.add_option('-s', '--slate',      dest='slate',       type  ='string',     default='',          help='Specify slate frame template')
Parser.add_option('--thumbnail',        dest='thumbnail',   action='store_true', default=False,       help='Add a thumbnail image on slate frame')
Parser.add_option('--addtime',          dest='addtime',     action='store_true', default=False,       help='Draw time with date')
Parser.add_option('--datesuffix',       dest='datesuffix',  action='store_true', default=False,       help='Add date suffix to output file name')
Parser.add_option('--timesuffix',       dest='timesuffix',  action='store_true', default=False,       help='Add time suffix to output file name')
Parser.add_option('-V', '--verbose',    dest='verbose',     action='store_true', default=False,       help='Verbose mode')
Parser.add_option('-D', '--debug',      dest='debug',       action='store_true', default=False,       help='Debug mode (verbose mode, no commands execution)')
Parser.add_option('-A', '--afanasy',    dest='afanasy',     action='store_true', default=False,       help='Send to Afanasy')
Parser.add_option('--afconvcap',        dest='afconvcap',   type  ='int',        default=-1,          help='Afanasy convert tasks capacity, -1=default')
Parser.add_option('--afenccap',         dest='afenccap',    type  ='int',        default=-1,          help='Afanasy encode task capacity, -1=default')
Parser.add_option('--afuser',           dest='afuser',      type  ='string',     default='',          help='Change Afanasy job user, ''=current')
Parser.add_option('--tmpdir',           dest='tmpdir',      type  ='string',     default='',          help='Temporary directory, if not specified, .makemovie+date will be used')
Parser.add_option('--tmpformat',        dest='tmpformat',   type  ='string',     default='tga',       help='Temporary images format')

# Options to makeframe:
Parser.add_option('-r', '--resolution', dest='resolution',  type  ='string',     default='',          help='Format: 768x576, if empty images format used')
Parser.add_option('-g', '--gamma',      dest='gamma',       type  ='float',      default=-1.0,        help='Apply gamma correction')
Parser.add_option('-q', '--quality',    dest='quality',     type  ='string',     default='',          help='Temporary image quality, or format options')
Parser.add_option('--company',          dest='company',     type  ='string',     default='',          help='Draw company')
Parser.add_option('--project',          dest='project',     type  ='string',     default='',          help='Draw project')
Parser.add_option('--shot',             dest='shot',        type  ='string',     default='',          help='Draw shot')
Parser.add_option('--ver',              dest='shotversion', type  ='string',     default='',          help='Draw shot version')
Parser.add_option('--artist',           dest='artist',      type  ='string',     default='',          help='Draw artist')
Parser.add_option('--activity',         dest='activity',    type  ='string',     default='',          help='Draw activity')
Parser.add_option('--comments',         dest='comments',    type  ='string',     default='',          help='Draw comments')
Parser.add_option('--font',             dest='font',        type  ='string',     default='',          help='Specify font)')
Parser.add_option('--logopath',         dest='logopath',    type  ='string',     default='',          help='Add a specified image')
Parser.add_option('--logosize',         dest='logosize',    type  ='int',        default=20,   	      help='Logotype size, percent of image')
Parser.add_option('--logograv',         dest='logograv',    type  ='string',     default='southeast', help='Logotype positioning gravity')
Parser.add_option('--draw169',          dest='draw169',     type  ='int',        default=0,           help='Draw 16:9 cacher opacity')
Parser.add_option('--draw235',          dest='draw235',     type  ='int',        default=0,           help='Draw 2.35 cacher opacity')
Parser.add_option('--line169',          dest='line169',     type  ='string',     default='',          help='Draw 16:9 line color: "255,255,0"')
Parser.add_option('--line235',          dest='line235',     type  ='string',     default='',          help='Draw 2.35 line color: "255,255,0"')
Parser.add_option('--stereo',           dest='stereo',      action='store_true', default=False,       help='Force stereo mode, if only one sequence provided')

(Options, args) = Parser.parse_args()

if len(args) < 2: Parser.error('Not enough arguments provided.')
if len(args) > 3: Parser.error('Too many arguments provided.')

MOVIEMAKER = os.path.dirname( sys.argv[0])
CODECSDIR  = os.path.join( MOVIEMAKER, 'codecs')
LOGOSDIR   = os.path.join( MOVIEMAKER, 'logos')

Inpattern1 = args[0]
Inpattern2 = ''
Output     = args[1]
Stereo = Options.stereo
if len(args) > 2:
   Inpattern2 = args[1]
   Output     = args[2]
   Stereo     = True

Codec       = Options.codec
Resolution  = Options.resolution
Options.logopath    = Options.logopath
Datesuffix  = Options.datesuffix
Timesuffix  = Options.timesuffix

Verbose     = Options.verbose
Debug       = Options.debug

TmpDir      = Options.tmpdir
TmpFormat   = Options.tmpformat

# Parameters initialization:
if Debug: Verbose = True
if Verbose: print 'VERBOSE MODE:'
if Debug: print 'DEBUG MODE:'


# Definitions:
tmpname   = 'img'
tmplogo   = 'logo.tga'

need_convert = False
if Stereo: need_convert = True
need_logo = False

# Check output folder:
if not os.path.isdir( os.path.dirname( Output)):
   print 'Output folder does not exist:'
   print Output
   sys.exit(1)

# Encode command:
Codec = Codec.lower()
encoder = Codec.split('.')
if len(encoder) < 2:
   encoder = 'ffmpeg'
   Codec += '.' + encoder
else: encoder = encoder[-1]
if Verbose: print 'Encoder engine = "%s"' % encoder
if os.path.dirname( Codec) == '': Codec = os.path.join( CODECSDIR, Codec)
if not os.path.isfile( Codec):
   print 'Can`t find codec "%s"' % Codec
   sys.exit(1)
file = open( Codec)
lines = file.readlines()
cmd_encode = lines[len(lines)-1].strip()
if len(cmd_encode) < 2:
   print 'Invalid encode file "%s"' % Codec
   sys.exit(1)
if Verbose: print 'Encode command = "%s"' % cmd_encode

# Date and time:
#datetimestring = '`date +%y-%m-%d_%H-%M`'
datetimestring = time.strftime('%y-%m-%d')
datetimesuffix = ''
if Datesuffix: datetimesuffix += time.strftime('%y%m%d')
if Options.addtime:
   if datetimestring != '': datetimestring += ' '
   datetimestring += time.strftime('%H:%M')
if Timesuffix:
   if datetimesuffix != '': datetimesuffix += '_'
   datetimesuffix += time.strftime('%H%M')

# Output file:
Output = Output.strip('" ')
afjobname = os.path.basename( Output)
if datetimesuffix != '': Output += '_' + datetimesuffix
if Codec.find('xvid') != -1: Output += '.avi'
else: Output += '.mov'
if Verbose: print 'Output = ' + Output

# Resolution:
Width = 0
Height = 0
if Resolution != '':
   need_convert = True
   pos = Resolution.find('x')
   if pos <= 0:
      print 'Invalid resolution specified.'
      sys.exit(1)
   Width = int(Resolution[ : pos ])
   Height = int(Resolution[ pos + 1 : ])
   if Verbose: print 'Output Resolution = %(Width)d x %(Height)d' % vars()
   afjobname += ' %s' % Resolution



# Get images function:
def getImages( inpattern):

   # Input directory:
   inputdir = os.path.dirname( inpattern)
   if Verbose: print 'InputDir = "%s"' % inputdir
   if not os.path.isdir( inputdir):
      print 'Can\'t find input directory "%s"' % inputdir
      sys.exit(1)

   # Input files pattern processing:
   pattern = os.path.basename( inpattern)
   digitspos = pattern.rfind('#')
   digitslen = 0
   if digitspos < 0:
      # Process %04d pattern:
      digitspos = pattern.rfind('%0')
      if digitspos < 0:
         print 'Can\'t find #### or %04d in input files pattern.'
         sys.exit(1)
      if pattern[digitspos+3] != 'd':
         print 'Invalid %04d pattern.'
         sys.exit(1)
      try:
         digitsnum = int(pattern[digitspos+2])
      except:
         print 'Unable to find number in %04d pattern.'
         sys.exit(1)
      digitslen = 4
   else:
      # Process #### pattern:
      digitsnum = 1
      for i in range(digitspos):
         if pattern[digitspos-digitsnum] == '#':
            digitsnum += 1
         else:
            break
      digitslen = digitsnum
      digitspos = digitspos - digitslen + 1
   prefix = pattern[ : digitspos ]
   suffix = pattern[ digitspos+digitslen :]

   # Input files search pattern:
   allFiles = []
   eprefix = re.escape( prefix)
   esuffix = re.escape( suffix)
   expr = r'%(eprefix)s([0-9]{%(digitsnum)s,%(digitsnum)s})%(esuffix)s' % vars()
   if Verbose: print 'Expression = ' + expr
   expr = re.compile( expr)
   allItems = os.listdir( inputdir)
   for item in allItems:
      if not os.path.isfile( os.path.join( inputdir, item)): continue
      if not expr.match( item): continue
      allFiles.append( os.path.join( inputdir, item))
   if len(allFiles) <= 1:
      print 'None or only one file founded matching pattern.'
      print 'Input directory:'
      print inputdir
      print '  prefix, digits, suffix = %(prefix)s, %(digitsnum)d, %(suffix)s' % vars()
      print 'Expression:'
      print expr.pattern
      sys.exit(1)
   allFiles.sort()
   if Verbose: print 'Files fonded: %d' % len(allFiles)

   # Input files indentify:
   afile = allFiles[0]
   identify = 'convert -identify "%s"'
   if sys.platform.find('win') == 0: identify += ' nul'
   else: identify += ' /dev/null'
   pipe = subprocess.Popen(identify % afile, shell=True, bufsize=100000, stdout=subprocess.PIPE).stdout
   identify = pipe.read()
   identify = identify.replace( afile, '')
   identify = identify.strip()
   if len(identify) < 1:
      print 'Invalid image "%s"' % afile
      sys.exit(1)
   identify = identify.split(' ')
   if len(identify) < 1:
      print 'Invalid image "%s"' % afile
      sys.exit(1)
   if Verbose: print 'Identify: %s' % identify
   imgtype = identify[0]
   if Verbose: print 'Images type = "%s"' % imgtype

   return allFiles, inputdir, prefix, digitsnum, suffix



# Call get images function:
images1, inputdir, prefix, digitsnum, suffix = getImages( Inpattern1)
if Inpattern2 != '':
   images2, inputdir, prefix, digitsnum, suffix = getImages( Inpattern2)
   if len(images1) != len(images2):
      print 'Error: Sequences lenght is not the same'
      sys.exit(1)

# Temporary directory:
if not Debug:
   if TmpDir == '':
      ftime = time.time()
      TmpDir = 'makemovie.' + time.strftime('%y-%m-%d_%H-%M-%S_') + str(ftime - int(ftime))[2:]
      if Options.afanasy:
         TmpDir = os.path.join( os.path.dirname(Output), '.' + TmpDir)
      else:
         tmp = os.getenv('TMPDIR', os.getenv('TMP', os.getenv('TEMP')))
         if tmp is None:
            if sys.platform.find('win') == 0: tmp = 'c:\\temp'
            else: tmp = '/tmp'
         TmpDir = os.path.join( tmp, TmpDir)
   if os.path.isdir( TmpDir): shutil.rmtree( TmpDir)
else:
   TmpDir = os.path.dirname( os.path.dirname( Inpattern1))
print 'Temporary Directory:'
print TmpDir

# Commands construction:
cmd_makeframe = os.path.join( os.path.dirname(sys.argv[0]), 'makeframe.py')
cmd_makeframe = 'python ' + cmd_makeframe

# Calculate frame range:
FrameRange = ''
digits1 = re.findall(r'\d+', images1[0])
digits2 = re.findall(r'\d+', images1[-1])
if digits1 is not None and digits2 is not None:
   if len(digits1) and len(digits2):
      FrameRange = "%s-%s" % (digits1[-1].lstrip('0'), digits2[-1].lstrip('0'))
      if FrameRange[0] == '-': FrameRange = '0' + FrameRange

# Construct frame conversion command arguments:
cmd_args = ''
if Options.resolution   != '': cmd_args += ' -r %s' % Options.resolution
if Options.quality      != '': cmd_args += ' -q %s' % Options.quality
if Options.company      != '': cmd_args += ' -c "%s"' % Options.company
if Options.project      != '': cmd_args += ' -p "%s"' % Options.project
if Options.artist       != '': cmd_args += ' -a "%s"' % Options.artist
if Options.shot         != '': cmd_args += ' -s "%s"' % Options.shot
if Options.shotversion  != '': cmd_args += ' --ver "%s"'       % Options.shotversion
if Options.font         != '': cmd_args += ' --font "%s"'      % Options.font
if Options.activity     != '': cmd_args += ' --activity "%s"'  % Options.activity
if Options.comments     != '': cmd_args += ' --comments "%s"'  % Options.comments
if FrameRange           != '': cmd_args += ' --framerange "%s"'  % FrameRange
if Stereo: cmd_args += ' --stereo'
cmd_args += ' -d "%s"' % datetimestring
cmd_args += ' -m "%s"' % os.path.basename(Output)

imgCount = 0
# Pre composition:
cmd_precomp = []
name_precomp = []

# Reformat logo command:
if Options.logopath != '':
   if need_convert:
      need_logo = True
      logopath = Options.logopath
      if not os.path.isfile( logopath):
         logopath = os.path.join( LOGOSDIR, logopath)
         if not os.path.isfile( logopath):
            print 'Can`t find logo "%s".' % logopath
            exit(1)
      logow = int( Width  * Options.logosize / 100 )
      logoh = int( Height * Options.logosize / 100 )
      tmplogo = os.path.join( TmpDir, tmplogo)
      cmd = 'convert'
      cmd += ' "%s"' % logopath
      cmd += ' -gravity %s -background black' % Options.logograv
      cmd += ' -resize %dx%d' % ( logow, logoh)
      cmd += ' -extent %dx%d' % ( Width-Width/15, Height-Height/15)
      cmd += ' "%s"' % tmplogo
   else:
      print 'Can\'t add logo if output resolution is not specified.'
      exit(1)
   cmd_precomp.append(cmd)
   name_precomp.append('Reformat logo')

# Generate convert commands lists:
cmd_convert = []
name_convert = []

# Generate header:
if need_convert and Options.slate != '':
   cmd = cmd_makeframe + cmd_args
   if need_logo: cmd += ' --logopath "%s"' % tmplogo
   cmd += ' --drawcolorbars' + cmd_args
   cmd += ' -t "%s"' % Options.slate
   cmd += ' "%s"' % images1[int(len(images1)/2)]
   if Inpattern2 != '': cmd += ' "%s"' % images2[int(len(images1)/2)]
   cmd += ' "%s"' % (os.path.join( TmpDir, tmpname) + '.%07d.' % imgCount + TmpFormat)
   cmd_convert.append(cmd)
   name_convert.append('Generate header')
   imgCount += 1

if need_convert:
   i = 0
   for afile in images1:
      cmd = cmd_makeframe + cmd_args
      if Options.template != '': cmd += ' -t "%s"'         % Options.template
      if Options.gamma     >  0: cmd += ' -g %.2f'         % Options.gamma
      if Options.draw169   >  0: cmd += ' --draw169 %d'    % Options.draw169
      if Options.draw235   >  0: cmd += ' --draw235 %d'    % Options.draw235
      if Options.line169  != '': cmd += ' --line169 "%s"'  % Options.line169
      if Options.line235  != '': cmd += ' --line235 "%s"'  % Options.line235

      cmd += ' "%s"' % afile
      if Inpattern2 != '':
         cmd += ' "%s"' % images2[i]
         i += 1
      cmd += ' "%s"' % (os.path.join( TmpDir, tmpname) + '.%07d.' % imgCount + TmpFormat)

      cmd_convert.append( cmd)
      name_convert.append( afile)
      imgCount += 1

# Encode commands:
auxargs = ''
if Codec != 'ffmpeg':
   inputmask = os.path.join( inputdir, prefix+'%0'+str(digitsnum)+'d'+suffix)
   if len(cmd_convert): inputmask = os.path.join( TmpDir, tmpname+'.%07d.'+TmpFormat)
elif Codec == 'mencoder':
   inputmask = os.path.join( inputdir, prefix+'*'+suffix)
   if len(cmd_convert): inputmask = os.path.join( TmpDir, tmpname+'.*.'+TmpFormat)
else:
   print 'Unknown encoder = "%s"' % encoder
   exit(1)
cmd_encode = cmd_encode.replace('@MOVIEMAKER@', MOVIEMAKER)
cmd_encode = cmd_encode.replace('@CODECS@',     CODECSDIR)
cmd_encode = cmd_encode.replace('@INPUT@',      inputmask)
cmd_encode = cmd_encode.replace('@FPS@',        Options.fps)
cmd_encode = cmd_encode.replace('@OUTPUT@',     Output)
cmd_encode = cmd_encode.replace('@ARGS@',       auxargs)

# Print commands:
if Debug:
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
   print
   os.system( cmd_encode)
   print
   sys.exit(0)

# Construct Afanasy job:
if Options.afanasy:
   af = __import__('af', globals(), locals(), [])
   j=af.Job( afjobname)
   if Options.afuser != '': j.setUserName( Options.afuser)

   if len(cmd_precomp):
      bp=af.Block( 'precomp', 'movgen')
      j.blocks.append( bp)
      n = 0
      for cmd in cmd_precomp:
         t=af.Task( name_precomp[n])
         bp.tasks.append( t)
         t.setCommand( cmd)
         n += 1
      bp.setCapacity( Options.afconvcap)

   if need_convert:
      bc=af.Block( 'convert', 'movgen')
      j.blocks.append( bc)
      n = 0
      for cmd in cmd_convert:
         t=af.Task( name_convert[n])
         bc.tasks.append( t)
         t.setCommand( cmd)
         n += 1
      bc.setCapacity( Options.afconvcap)
      if need_logo: bc.setDependMask('precomp')
      bc.setTasksMaxRunTime(11)

   be = af.Block( 'encode', 'movgen')
   j.blocks.append( be)
   t = af.Task( Output)
   be.tasks.append( t)
   t.setCommand( cmd_encode)
   be.setCapacity( Options.afenccap)
   if need_convert:
      be.setDependMask('convert')
      j.setCmdPre( 'mkdir "%s" && chmod a+rwx "%s"' % (os.path.abspath(TmpDir),os.path.abspath(TmpDir)))
      j.setCmdPost('rm -rf "%s"' % os.path.abspath(TmpDir))

   if Verbose: j.output(1)

# Commands execution:
if Options.afanasy: j.send( Verbose)
else:
   if len(cmd_precomp) or need_convert: os.mkdir(TmpDir, 0777)
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
   if not Debug:
      if os.path.isdir( TmpDir):
         shutil.rmtree( TmpDir)
         if os.path.isdir( TmpDir):
            print 'Warning: Temporary directory still exsists:'
            print TmpDir
      else:
         print 'Warning: Temporary directory does not exsist:'
         print TmpDir
   print
   print 'Done'
