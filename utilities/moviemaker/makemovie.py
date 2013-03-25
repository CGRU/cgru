#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import re
import time
import shutil
import signal
import subprocess

print('MakeMovie: "%s"' % __file__)

TmpDir = ''
def rmdir( signum, frame):
   print('\nInterrupt received...')
   if not Debug:
      if os.path.isdir( TmpDir):
         shutil.rmtree( TmpDir)
         if os.path.isdir( TmpDir):
            print('Warning: Temporary directory still exsists:')
            print( TmpDir)
      else:
         print('Warning: Temporary directory does not exsist:')
         print( TmpDir)
   exit(0)

signal.signal(signal.SIGTERM, rmdir)
signal.signal(signal.SIGABRT, rmdir)
signal.signal(signal.SIGINT,  rmdir)

from optparse import OptionParser
Parser = OptionParser(usage="%prog [options] input_files_pattern(s)] output\n\
   Pattern examples = \"img.####.jpg\" or \"img.%04d.jpg\".\n\
   Type \"%prog -h\" for help", version="%prog 1.0")

Parser.add_option('-c', '--codec',      dest='codec',       type  ='string',     default='photojpg_best.ffmpeg', help='File with encode command line in last line')
Parser.add_option('-f', '--fps',        dest='fps',         type  ='string',     default='25',        help='Frames per second')
Parser.add_option('-t', '--template',   dest='template',    type  ='string',     default='',          help='Specify frame template to use')
Parser.add_option('-s', '--slate',      dest='slate',       type  ='string',     default='',          help='Specify slate frame template')
Parser.add_option('-n', '--container',  dest='container',   type  ='string',     default='mov',       help='Container')
Parser.add_option('--fs',               dest='framestart',  type  ='int',        default=-1,          help='First frame to use, -1 use the first founded')
Parser.add_option('--fe',               dest='frameend',    type  ='int',        default=-1,          help='Last frame to use, -1 use the last founded')
Parser.add_option('--fffirst',          dest='fffirst',     action='store_true', default=False,       help='Draw first frame as first and not actual frame number.')
Parser.add_option('--thumbnail',        dest='thumbnail',   action='store_true', default=False,       help='Add a thumbnail image on slate frame')
Parser.add_option('--addtime',          dest='addtime',     action='store_true', default=False,       help='Draw time with date')
Parser.add_option('--datesuffix',       dest='datesuffix',  action='store_true', default=False,       help='Add date suffix to output file name')
Parser.add_option('--timesuffix',       dest='timesuffix',  action='store_true', default=False,       help='Add time suffix to output file name')
Parser.add_option('--faketime',         dest='faketime',    type  ='int',        default=0,           help='Use fake time')
Parser.add_option('-V', '--verbose',    dest='verbose',     action='store_true', default=False,       help='Verbose mode')
Parser.add_option('-D', '--debug',      dest='debug',       action='store_true', default=False,       help='Debug mode (verbose mode, no commands execution)')
Parser.add_option('-A', '--afanasy',    dest='afanasy',     action='store_true', default=False,       help='Send to Afanasy')
Parser.add_option('--afconvcap',        dest='afconvcap',   type  ='int',        default=-1,          help='Afanasy convert tasks capacity, -1=default')
Parser.add_option('--afenccap',         dest='afenccap',    type  ='int',        default=-1,          help='Afanasy encode task capacity, -1=default')
Parser.add_option('--afuser',           dest='afuser',      type  ='string',     default='',          help='Change Afanasy job user, ''=current')
Parser.add_option('--tmpdir',           dest='tmpdir',      type  ='string',     default='',          help='Temporary directory, if not specified, .makemovie+date will be used')
Parser.add_option('--tmpformat',        dest='tmpformat',   type  ='string',     default='tga',       help='Temporary images format')
Parser.add_option('--tmpquality',       dest='tmpquality',  type  ='string',     default='',          help='Temporary image quality, or format options')
Parser.add_option('--audio',            dest='audio',       type  ='string',     default='',          help='Add sound from audio file')
Parser.add_option('--afreq',            dest='afreq',       type  ='int',        default=22000,       help='Audio frequency')
Parser.add_option('--akbits',           dest='akbits',      type  ='int',        default=128,         help='Audio kilo bits rate')
Parser.add_option('--acodec',           dest='acodec',      type  ='string',     default='libmp3lame',help='Audio codec')

# Options to makeframe:
Parser.add_option('-r', '--resolution', dest='resolution',     type  ='string',     default='',          help='Format: 768x576, if empty images format used')
Parser.add_option('-g', '--gamma',      dest='gamma',          type  ='float',      default=-1.0,        help='Apply gamma correction')
Parser.add_option('--aspect_in',        dest='aspect_in',      type  ='float',      default=-1.0,        help='Input image aspect, -1 = no changes')
Parser.add_option('--aspect_auto',      dest='aspect_auto',    type  ='float',      default=-1.0,        help='Auto image aspect (2 if w/h <= aspect_auto), -1 = no changes')
Parser.add_option('--aspect_out',       dest='aspect_out',     type  ='float',      default=-1.0,        help='Output movie aspect, "-1" = no changes')
Parser.add_option('--noautocorr',       dest='noautocorr',     action='store_true', default=False,       help='Disable auto color correction for Cineon and EXR')
Parser.add_option('--correction',       dest='correction',     type  ='string',     default='',          help='Add custom color correction parameters')
Parser.add_option('--company',          dest='company',        type  ='string',     default='',          help='Draw company')
Parser.add_option('--project',          dest='project',        type  ='string',     default='',          help='Draw project')
Parser.add_option('--shot',             dest='shot',           type  ='string',     default='',          help='Draw shot')
Parser.add_option('--ver',              dest='shotversion',    type  ='string',     default='',          help='Draw shot version')
Parser.add_option('--artist',           dest='artist',         type  ='string',     default='',          help='Draw artist')
Parser.add_option('--activity',         dest='activity',       type  ='string',     default='',          help='Draw activity')
Parser.add_option('--comments',         dest='comments',       type  ='string',     default='',          help='Draw comments')
Parser.add_option('--font',             dest='font',           type  ='string',     default='',          help='Specify font)')
Parser.add_option('--lgspath',          dest='lgspath',        type  ='string',     default='',          help='Slate logotype path')
Parser.add_option('--lgssize',          dest='lgssize',        type  ='int',        default=25,   	      help='Slate logotype size, percent of image')
Parser.add_option('--lgsgrav',          dest='lgsgrav',        type  ='string',     default='southeast', help='Slate logotype positioning gravity')
Parser.add_option('--lgfpath',          dest='lgfpath',        type  ='string',     default='',          help='Flrame logotype path')
Parser.add_option('--lgfsize',          dest='lgfsize',        type  ='int',        default=10,   	      help='Flrame logotype size, percent of image')
Parser.add_option('--lgfgrav',          dest='lgfgrav',        type  ='string',     default='north',     help='Flrame logotype positioning gravity')
Parser.add_option('--cacher_aspect',    dest='cacher_aspect',  type  ='float',      default=1.85,        help='Cacher aspect')
Parser.add_option('--cacher_opacity',   dest='cacher_opacity', type  ='int',        default=0,           help='Cacher opacity')
Parser.add_option('--line_aspect',      dest='line_aspect',    type  ='float',      default=1.85,        help='Cacher line aspect')
Parser.add_option('--line_color',       dest='line_color',     type  ='string',     default='',          help='Cacher line opacity')
Parser.add_option('--draw169',          dest='draw169',        type  ='int',        default=0,           help='Draw 16:9 cacher opacity')
Parser.add_option('--draw235',          dest='draw235',        type  ='int',        default=0,           help='Draw 2.35 cacher opacity')
Parser.add_option('--line169',          dest='line169',        type  ='string',     default='',          help='Draw 16:9 line color: "255,255,0"')
Parser.add_option('--line235',          dest='line235',        type  ='string',     default='',          help='Draw 2.35 line color: "255,255,0"')
Parser.add_option('--createoutdir',     dest='createoutdir',   action='store_true', default=False,       help='Create output folder if it not exists')
Parser.add_option('--stereodub',        dest='stereodub',      action='store_true', default=False,       help='Force stereo mode, if only one sequence provided')

(Options, args) = Parser.parse_args()

if len(args) < 2: Parser.error('Not enough arguments provided.')
if len(args) > 3: Parser.error('Too many arguments provided.')

MOVIEMAKER = os.path.dirname( sys.argv[0])
CODECSDIR  = os.path.join( MOVIEMAKER, 'codecs')
LOGOSDIR   = os.path.join( MOVIEMAKER, 'logos')

Inpattern1 = args[0]
Inpattern2 = ''
Output     = args[1]
Stereo = Options.stereodub
if len(args) > 2:
   Inpattern2 = args[1]
   Output     = args[2]
   Stereo     = True

print( Inpattern1 + ' ' + Inpattern2)

Codec       = Options.codec
AspectIn    = Options.aspect_in
Datesuffix  = Options.datesuffix
Timesuffix  = Options.timesuffix
Audio       = Options.audio

Verbose     = Options.verbose
Debug       = Options.debug

TmpDir      = Options.tmpdir
TmpFormat   = Options.tmpformat

# Parameters initialization:
if Debug: Verbose = True
if Verbose: print('VERBOSE MODE:')
if Debug: print('DEBUG MODE:')


# Definitions:
tmpname   = 'img'
tmplgs    = 'logo_slate.tga'
tmplgf    = 'logo_frame.tga'

need_convert = False
if Stereo: need_convert = True

# Check frame range:
if Options.framestart != -1 and Options.frameend != -1:
   if Options.framestart > Options.frameend:
      print('Error: First frame %d > last frame %d' % (Options.framestart, Options.frameend))
      sys.exit(1)

# Check output folder:
if os.path.dirname( Output) != '' and not os.path.isdir( os.path.dirname( Output)):
   if Options.createoutdir:
      os.makedirs( os.path.dirname( Output))
   else:
      print('Output folder does not exist:')
      print( os.path.dirname( Output))
      sys.exit(1)

# Encode command:
# Codec = Codec.lower()
encoder = Codec.split('.')
if len(encoder) < 2:
   encoder = 'ffmpeg'
   Codec += '.' + encoder
else: encoder = encoder[-1]
if Verbose: print('Encoder engine = "%s"' % encoder)
if os.path.dirname( Codec) == '': Codec = os.path.join( CODECSDIR, Codec)
if not os.path.isfile( Codec):
   print('Can`t find codec "%s"' % Codec)
   sys.exit(1)
file = open( Codec)
lines = file.readlines()
cmd_encode = lines[len(lines)-1].strip()
if len(cmd_encode) < 2:
   print('Invalid encode file "%s"' % Codec)
   sys.exit(1)
if Verbose: print('Encode command = "%s"' % cmd_encode)

# Date and time:
localtime = time.localtime()
if Options.faketime : localtime = time.localtime( 1.0 * Options.faketime)
datetimestring = time.strftime('%y-%m-%d', localtime)
datetimesuffix = ''
if Datesuffix: datetimesuffix += time.strftime('%y%m%d', localtime)
if Options.addtime:
   if datetimestring != '': datetimestring += ' '
   datetimestring += time.strftime('%H:%M', localtime)
if Timesuffix:
   if datetimesuffix != '': datetimesuffix += '_'
   datetimesuffix += time.strftime('%H%M', localtime)

# Output file:
Output = Output.strip('" ')
afjobname = os.path.basename( Output)
if datetimesuffix != '': Output += '_' + datetimesuffix
if Verbose: print('Output = ' + Output)

# Options.resolution:
Width = 0
Height = 0
if Options.resolution != '':
   need_convert = True
   res = Options.resolution.split('x')
   if len(res) < 2: Parser.error('Invalid resolution specified.')
   Width = int(res[0])
   Height = int(res[1])
   if len(res) > 2 and Options.aspect_out < 0: Options.aspect_out = float(res[2])
   if Verbose: print('Output Resolution = %dx%dx%f' % ( Width, Height, Options.aspect_out))
   afjobname += ' %s' % Options.resolution


# Get images function:
def getImages( inpattern):

   # Input directory:
   inputdir = os.path.dirname( inpattern)
   if Verbose: print('InputDir = "%s"' % inputdir)
   if not os.path.isdir( inputdir):
      print('Can\'t find input directory "%s"' % inputdir)
      sys.exit(1)

   # Input files pattern processing:
   pattern = os.path.basename( inpattern)
   # Process %d pattern:
   digitsall = re.findall(r'%\d{,}d', pattern)
   if len(digitsall):
      digitsall = digitsall[-1]
      digitspos = pattern.rfind( digitsall)
      digitslen = len(digitsall)
      if len(digitsall) > 2:
         # Process %0#d pattern:
         try:
            padding = int(digitsall[1:-1])
         except:
            print('Unable to find number in %#d pattern ("'+ digitsall[1:-1] +'" not a number).')
            sys.exit(1)
      else:
         # Process %d pattern:
         padding = -1
   else:
      # Process # pattern:
      digitsall = re.findall(r'#{1,}', pattern)
      if len(digitsall):
         digitsall = digitsall[-1]
         digitspos = pattern.rfind( digitsall)
         digitslen = len(digitsall)
         padding = digitslen
      else:
         print('Can\'t find #### or %0#d in input files pattern.')
         sys.exit(1)

   prefix = pattern[ : digitspos ]
   suffix = pattern[ digitspos+digitslen :]
   
   print('Images prefix, padding, suffix = "%s" %d "%s"' % (prefix,padding,suffix))

   # Input files search pattern:
   allFiles = []
   eprefix = re.escape( prefix)
   esuffix = re.escape( suffix)
   if padding > 1:
      expr = r'%(eprefix)s([0-9]{%(padding)s,%(padding)s})%(esuffix)s$' % vars()
   else:
      expr = r'%(eprefix)s([0-9]{1,})%(esuffix)s$' % vars()
   if Verbose: print('Expression = ' + expr)
   expr = re.compile( expr)
   allItems = os.listdir( inputdir)
   for item in allItems:
      if not os.path.isfile( os.path.join( inputdir, item)): continue
      if expr.match( item) is None: continue
      if Options.framestart != -1 or Options.frameend != -1:
         if padding > 1:
            frame = int( item[digitspos:digitspos+padding])
         else:
            frame = int(re.findall(r'\d{1,}', item)[-1])
         if frame < Options.framestart or frame > Options.frameend: continue
      allFiles.append( os.path.join( inputdir, item))
   if len(allFiles) <= 1:
      print('None or only one file founded matching pattern.')
      print('Input directory:')
      print( inputdir)
      print('Expression:')
      print( expr.pattern)
      if Options.framestart != -1 or Options.frameend != -1:
         print('Frame Range: %d - %d' % (Options.framestart, Options.frameend))
      sys.exit(1)
   allFiles.sort()
   if Verbose: print('Files fonded: %d' % len(allFiles))

   # Input files indentify:
   afile = allFiles[0]
   identify = 'convert -identify "%s"'
   if sys.platform.find('win') == 0: identify += ' nul'
   else: identify += ' /dev/null'
   pipe = subprocess.Popen(identify % afile, shell=True, bufsize=100000, stdout=subprocess.PIPE).stdout
   identify = pipe.read()
   if not isinstance( identify, str): identify = str( identify, 'utf-8')
   identify = identify.replace( afile, '')
   identify = identify.strip()
   if len(identify) < 1:
      print('Invalid image "%s"' % afile)
      sys.exit(1)
   identify = identify.split(' ')
   print( identify)
   if len(identify) < 1:
      print('Invalid image "%s"' % afile)
      sys.exit(1)
   if Verbose: print('Identify: %s' % identify)
   imgtype = identify[0]
   imgres = identify[1]
   imgres = imgres.split('x')
   imgresx = int(imgres[0])
   imgresy = int(imgres[1])
   if Verbose:
      print('Images type = "%s"' % imgtype)
      print('Images resolution = %dx%d' % (imgresx , imgresy))
   global AspectIn
   if Options.aspect_auto > 0:
      if float(imgresx) / float(imgresy) < Options.aspect_auto:
         AspectIn = 2.0
         print('Auto AspectIn = %f (%f)' % (AspectIn, float(imgresx) / float(imgresy)))
   elif Verbose:
      print('AspectIn = %f' % AspectIn)

   return allFiles, inputdir, prefix, padding, suffix



# Call get images function:
images1, inputdir, prefix, padding, suffix = getImages( Inpattern1)
if Inpattern2 != '':
   images2, inputdir, prefix, padding, suffix = getImages( Inpattern2)
   if len(images1) != len(images2):
      print('Error: Sequences lenght is not the same')
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
print('Temporary Directory:')
print( TmpDir)

# Commands construction:
cmd_makeframe = os.path.join( os.path.dirname(sys.argv[0]), 'makeframe.py')
cmd_makeframe = '"%s" "%s"' % ( os.getenv('CGRU_PYTHONEXE','python'), cmd_makeframe)

# Calculate frame range:
FrameRange = ''
FramePadding = 0
digits1 = re.findall(r'\d+', images1[0])
if digits1 is not None and len(digits1): FramePadding = len(digits1[-1])
if Options.fffirst:
   FrameRange = '1-' + str(len(images1))
else:
   digits2 = re.findall(r'\d+', images1[-1])
   if digits1 is not None and digits2 is not None:
      if len(digits1) and len(digits2):
         FrameRange = "%s-%s" % (digits1[-1].lstrip('0'), digits2[-1].lstrip('0'))
         if FrameRange[0] == '-': FrameRange = '0' + FrameRange

# Construct frame conversion command arguments:
cmd_args = ''
if Options.resolution   != '': cmd_args += ' -r %s' % Options.resolution
if AspectIn              >  0: cmd_args += ' --aspect_in %f' % AspectIn
if Options.aspect_out    >  0: cmd_args += ' --aspect_out %f' % Options.aspect_out
if Options.tmpquality   != '': cmd_args += ' -q %s' % Options.tmpquality
if Options.company      != '': cmd_args += ' -c "%s"' % Options.company
if Options.project      != '': cmd_args += ' -p "%s"' % Options.project
if Options.artist       != '': cmd_args += ' -a "%s"' % Options.artist
if Options.shot         != '': cmd_args += ' -s "%s"' % Options.shot
if Options.shotversion  != '': cmd_args += ' --ver "%s"'       % Options.shotversion
if Options.font         != '': cmd_args += ' --font "%s"'      % Options.font
if Options.activity     != '': cmd_args += ' --activity "%s"'  % Options.activity
if Options.comments     != '': cmd_args += ' --comments "%s"'  % Options.comments
if Options.correction   != '': cmd_args += ' --correction "%s"' % Options.correction
if FrameRange           != '': cmd_args += ' --framerange "%s"'  % FrameRange
if Options.cacher_opacity > 0:
   cmd_args += ' --cacher_aspect %f' % Options.cacher_aspect
   cmd_args += ' --cacher_opacity %d' % Options.cacher_opacity
if Options.line_color != '':
   cmd_args += ' --line_aspect %f' % Options.line_aspect
   cmd_args += ' --line_color "%s"' % Options.line_color
if Stereo: cmd_args += ' --stereodub'
cmd_args += ' -d "%s"' % datetimestring
cmd_args += ' -m "%s"' % os.path.basename(Output)

imgCount = 0
# Pre composition:
cmd_precomp = []
name_precomp = []

# Extract audio track(s) from file to flac if it is not flac already:
if len(Audio):
   if not os.path.isfile( Audio):
      print('Error: Audio file does not exist:')
      print( Audio)
      exit(1)
   if len(Audio) >= 5:
      if Audio[-5:] != '.flac':
         cmd_precomp.append('ffmpeg -y -i "%s" -vn -acodec flac "%s.flac"' % (Audio,Audio))
         name_precomp.append('Audio "%s"' % os.path.basename(Audio))
         Audio += '.flac'

# Reformat logo:
logopath = [ Options.lgspath, Options.lgfpath ]
logosize = [ Options.lgssize, Options.lgfsize ]
logograv = [ Options.lgsgrav, Options.lgfgrav ]
tmplogo  = [          tmplgs,          tmplgf ]
logoname = [         'slate',         'frame' ]
for i in range(2):
   if logopath[i] != '':
      if need_convert:
         if not os.path.isfile( logopath[i]):
            logopath[i] = os.path.join( LOGOSDIR, logopath[i])
            if not os.path.isfile( logopath[i]):
               print('Can`t find logo "%s".' % logopath[i])
               exit(1)
         logow = int( Width  * logosize[i] / 100 )
         logoh = int( Height * logosize[i] / 100 )
         cmd = 'convert'
         cmd += ' "%s"' % logopath[i]
         cmd += ' -gravity %s -background black' % logograv[i]
         cmd += ' -resize %dx%d' % ( logow, logoh)
         cmd += ' -extent %dx%d' % ( Width, Height)
         tmplogo[i] = os.path.join( TmpDir, tmplogo[i])
         cmd += ' "%s"' % tmplogo[i]
      else:
         print('Can\'t add logo if output resolution is not specified.')
         exit(1)
      cmd_precomp.append(cmd)
      name_precomp.append('Reformat %s logo' % logoname[i])

# Generate convert commands lists:
cmd_convert = []
name_convert = []

# Generate header:
if need_convert and Options.slate != '':
   cmd = cmd_makeframe + cmd_args
   if Options.lgspath != '': cmd += ' --logopath "%s"' % tmplogo[0]
   cmd += ' --drawcolorbars' + cmd_args
   cmd += ' -t "%s"' % Options.slate
   cmd += ' "%s"' % images1[int(len(images1)/2)]
   if Inpattern2 != '': cmd += ' "%s"' % images2[int(len(images1)/2)]
   cmd += ' "%s"' % (os.path.join( TmpDir, tmpname) + '.%07d.' % imgCount + TmpFormat)
   cmd_convert.append(cmd)
   name_convert.append('Generate header')
   imgCount += 1

# Generate sequence frames:
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
      if Options.lgfpath  != '': cmd += ' --logopath "%s"' % tmplogo[1]
      if Options.noautocorr    : cmd += ' --noautocorr'
      if Options.fffirst:
         if FramePadding > 1:
            framestring = '%0' + str(FramePadding) + 'd'
            framestring = framestring % (i+1)
         else:
            framestring = str(i+1)
         cmd += ' -f "%s"' % framestring

         print( i, FramePadding, framestring)

      cmd += ' "%s"' % afile
      if Inpattern2 != '': cmd += ' "%s"' % images2[i]
      cmd += ' "%s"' % (os.path.join( TmpDir, tmpname) + '.%07d.' % imgCount + TmpFormat)

      cmd_convert.append( cmd)
      name_convert.append( afile)
      imgCount += 1
      i += 1

# Encode commands:
auxargs = ''
if encoder == 'ffmpeg' or encoder == 'nuke':
   inputmask = os.path.join( inputdir, prefix+'%0'+str(padding)+'d'+suffix)
   if len(cmd_convert): inputmask = os.path.join( TmpDir, tmpname+'.%07d.'+TmpFormat)
elif encoder == 'mencoder':
   inputmask = os.path.join( inputdir, prefix+'*'+suffix)
   if len(cmd_convert): inputmask = os.path.join( TmpDir, tmpname+'.*.'+TmpFormat)
else:
   print('Unknown encoder = "%s"' % encoder)
   exit(1)
if len(Audio) and encoder == 'ffmpeg':
   inputmask += '" -i "%s"' % Audio
   inputmask += ' -ar %d' % Options.afreq
   inputmask += ' -ab %dk' % Options.akbits
   inputmask += ' -acodec "%s' % Options.acodec

cmd_encode = cmd_encode.replace('@MOVIEMAKER@', MOVIEMAKER)
cmd_encode = cmd_encode.replace('@CODECS@',     CODECSDIR)
cmd_encode = cmd_encode.replace('@INPUT@',      inputmask)
cmd_encode = cmd_encode.replace('@FPS@',        Options.fps)
cmd_encode = cmd_encode.replace('@CONTAINER@',  Options.container)
cmd_encode = cmd_encode.replace('@OUTPUT@',     Output)
cmd_encode = cmd_encode.replace('@ARGS@',       auxargs)

# Print commands:
if Debug:
   if len(cmd_precomp):
      print('Precomp  first and last commands:\n')
      print(cmd_precomp[0])
      os.system( cmd_precomp[0])
      if len(cmd_precomp) > 1:
         print('...')
         print( cmd_precomp[-1])
         os.system( cmd_precomp[-1])
      print
   if need_convert:
      print('Convert first and last commands:\n')
      print( cmd_convert[0])
      os.system( cmd_convert[0])
      print('...')
      print( cmd_convert[-1])
      os.system( cmd_convert[-1])
      print
   print('Encode command:\n')
   print( cmd_encode + '\n')
   os.system( cmd_encode)
   print('\n')
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
      if Options.afconvcap > 0: bp.setCapacity( Options.afconvcap)

   if need_convert:
      bc=af.Block( 'convert', 'movgen')
      j.blocks.append( bc)
      n = 0
      for cmd in cmd_convert:
         t=af.Task( name_convert[n])
         bc.tasks.append( t)
         t.setCommand( cmd)
         n += 1
      if Options.afconvcap > 0: bc.setCapacity( Options.afconvcap)
      if len(cmd_precomp): bc.setDependMask('precomp')
      bc.setTasksMaxRunTime(11)

   be = af.Block( 'encode', 'movgen')
   j.blocks.append( be)
   t = af.Task( Output)
   be.tasks.append( t)
   t.setCommand( cmd_encode)
   if Options.afenccap > 0: be.setCapacity( Options.afenccap)
   if need_convert:
      be.setDependMask('convert')
      os.makedirs( TmpDir)
      j.setCmdPost('deletefiles "%s"' % os.path.abspath(TmpDir))

   if Verbose: j.output(1)

# Commands execution:
if Options.afanasy: j.send( Verbose)
else:
   if len(cmd_precomp) or need_convert:
      os.mkdir( TmpDir, 0o777)
   if len(cmd_precomp):
      n = 0
      print('ACTIVITY: Precomp')
      sys.stdout.flush()
      for cmd in cmd_precomp:
         print( name_precomp[n])
         os.system( cmd)
         n += 1
      print('')
   if need_convert:
      n = 0
      print('ACTIVITY: Convert')
      sys.stdout.flush()
      for cmd in cmd_convert:
         print( name_convert[n])
#         print(cmd)
#         output = subprocess.Popen( cmd, stdout=subprocess.PIPE).communicate()[0]
         cmd_array = []
#         if os.platform.find('win')
         subprocess.Popen( cmd, shell=True).communicate()
#         subprocess.Popen(['bash','-c',cmd]).communicate()
#         os.system( cmd)
#         print( output)
         n += 1
         print('PROGRESS: %d' % (100.0 * n / imgCount) + '%')
         sys.stdout.flush()
      print('')
   print('ACTIVITY: Encode')
   sys.stdout.flush()
   os.system( cmd_encode)
   if not Debug:
      if os.path.isdir( TmpDir):
         shutil.rmtree( TmpDir)
         if os.path.isdir( TmpDir):
            print('Warning: Temporary directory still exsists:')
            print( TmpDir)
      else:
         print('Warning: Temporary directory does not exsist:')
         print( TmpDir)
   print('')
   print('Done')
