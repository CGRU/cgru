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
   exit(0)

signal.signal(signal.SIGTERM, rmdir)
signal.signal(signal.SIGABRT, rmdir)
signal.signal(signal.SIGINT,  rmdir)

from optparse import OptionParser
parser = OptionParser(usage="%prog [options]\ntype \"%prog -h\" for help", version="%prog 1.0")
parser.add_option('-f', '--fps',        dest='fps',         type  ='int',        default=25,          help='Frames per second')
parser.add_option('-c', '--codec',      dest='codec',       type  ='string',     default='aphotojpg',  help='File with encode command line in last line')
parser.add_option('-r', '--resolution', dest='resolution',  type  ='string',     default='',          help='Format: 768x576, if empty images format used')
parser.add_option('-i', '--inpattern',  dest='inpattern',   type  ='string',     default='',          help='Input files pattern: img.####.jpg')
parser.add_option('-o', '--output',     dest='output',      type  ='string',     default='',          help='Output filename, if not specified, pattern will be used')
parser.add_option('-g', '--gamma',      dest='gamma',       type  ='float',      default=-1.0,        help='Apply gamma correction')
parser.add_option('--company',          dest='company',     type  ='string',     default='',          help='Draw company')
parser.add_option('--project',          dest='project',     type  ='string',     default='',          help='Draw project')
parser.add_option('--shot',             dest='shot',        type  ='string',     default='',          help='Draw shot')
parser.add_option('--shotversion',      dest='shotversion', type  ='string',     default='',          help='Draw shot version')
parser.add_option('--artist',           dest='artist',      type  ='string',     default='',          help='Draw artist')
parser.add_option('--activity',         dest='activity',    type  ='string',     default='',          help='Draw activity')
parser.add_option('--comments',         dest='comments',    type  ='string',     default='',          help='Draw comments')
parser.add_option('--font',             dest='font',        type  ='string',     default='',          help='Specify font)')
parser.add_option('--logopath',         dest='logopath',    type  ='string',     default='',          help='Add a specified image')
parser.add_option('--logosize',         dest='logosize',    type  ='string',     default='200x100',   help='Logotype size')
parser.add_option('--drawdate',         dest='drawdate',    action='store_true', default=False,       help='Draw date')
parser.add_option('--drawtime',         dest='drawtime',    action='store_true', default=False,       help='Draw time')
parser.add_option('--drawframe',        dest='drawframe',   action='store_true', default=False,       help='Draw frame')
parser.add_option('--drawfilename',     dest='drawfilename',action='store_true', default=False,       help='Draw file name')
parser.add_option('--draw169',          dest='draw169',     type  ='int',        default=0,           help='Draw 16:9 cacher opacity')
parser.add_option('--draw235',          dest='draw235',     type  ='int',        default=0,           help='Draw 2.35 cacher opacity')
parser.add_option('--datesuffix',       dest='datesuffix',  action='store_true', default=False,       help='Add date suffix to output file name')
parser.add_option('--timesuffix',       dest='timesuffix',  action='store_true', default=False,       help='Add time suffix to output file name')
parser.add_option('-V', '--verbose',    dest='verbose',     action='store_true', default=False,       help='Verbose mode')
parser.add_option('-D', '--debug',      dest='debug',       action='store_true', default=False,       help='Debug mode (verbose mode, no commands execution)')
parser.add_option('-A', '--afanasy',    dest='afanasy',     action='store_true', default=False,       help='Send to Afanasy')
parser.add_option('--afconvcap',        dest='afconvcap',   type  ='int',        default=10,          help='Afanasy convert tasks capacity')
parser.add_option('--afenccap',         dest='afenccap',    type  ='int',        default=500,         help='Afanasy encode task capacity')
parser.add_option('-u', '--username',   dest='username',    type  ='string',     default='',          help='Change Afanasy job user')
parser.add_option('--tmpdir',           dest='tmpdir',      type  ='string',     default='',          help='Temporary directory, if not specified, .makeMovie+date will be used')

(options, args) = parser.parse_args()

fps            = options.fps
codec          = options.codec
resolution     = options.resolution
inpattern      = options.inpattern
output         = options.output
drawdate       = options.drawdate
drawtime       = options.drawtime
drawframe      = options.drawframe
drawfilename   = options.drawfilename
draw169        = options.draw169
draw235        = options.draw235
font           = options.font
logopath       = options.logopath
logosize       = options.logosize
company        = options.company
project        = options.project
shot           = options.shot
shotversion    = options.shotversion
artist         = options.artist
activity       = options.activity
comments       = options.comments
datesuffix     = options.datesuffix
timesuffix     = options.timesuffix
gamma          = options.gamma

verbose     = options.verbose
debug       = options.debug

afanasy     = options.afanasy
afconvcap   = options.afconvcap
afenccap    = options.afenccap
username    = options.username

tmpdir      = options.tmpdir

# Parameters initialization:
if debug: verbose = True
if verbose: print 'VERBOSE MODE:'
if debug: print 'DEBUG MODE:'


# Definitions:
tmpName   = 'img'
tmpFormat = 'jpg'
tmpLogo   = 'logo.png'

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
datetimestring = ''
datetimesuffix = ''
if drawdate  : datetimestring += time.strftime('%y-%m-%d')
if datesuffix: datetimesuffix += time.strftime('%y%m%d')
if drawtime:
   if datetimestring != '': datetimestring += ' '
   datetimestring += time.strftime('%H:%M')
if timesuffix:
   if datetimesuffix != '': datetimesuffix += '_'
   datetimesuffix += time.strftime('%H%M')
#if re.match( r'win.*', sys.platform) != None: datetimestring = '%DATE%'
#datetimestring = "`python -c \"import time;print time.strftime('%y-%m-%d_%H-%M')\"`"

# Check required parameters:
if inpattern == '': parser.error('Input files not specified.')

# Input directory:
inputdir = os.path.dirname( inpattern)
if verbose: print 'InputDir = "%s"' % inputdir
if not os.path.isdir( inputdir):
   print 'Can\'t find input directory'
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
   exit(1)
if len(allFiles) == 1:
   print 'Founded only 1 file matching pattern.'
   exit(1)
allFiles.sort()
#print allFiles
if verbose: print 'Files fonded: %d' % len(allFiles)

# Input files indentify:
afile = os.path.join( inputdir, allFiles[0])
pipe = subprocess.Popen( 'identify '+afile, shell=True, bufsize=100000, stdout=subprocess.PIPE).stdout
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

# Input files correction:
correction = ''
corr_sRGB = ' -gamma 2.2'
corr_Log = ' -level 9%,67%,.6'
if   imgtype == 'EXR': correction = corr_sRGB
elif imgtype == 'DPX': correction = corr_Log
elif imgtype == 'CIN': correction = corr_Log

# Output file:
if output == '': output = os.path.join( os.path.dirname( inputdir), prefix.strip('_. '))
afjobname = os.path.basename( output)
if datetimesuffix != '': output += '_' + datetimesuffix
if codec.find('xvid') != -1: output += '.avi'
else: output += '.mov'
if debug: output = os.path.basename( output)
if verbose: print 'Output = ' + output
afjobname += ' %s' % codec

# Temporary directory:
if not debug:
   if tmpdir == '': tmpdir = os.path.join( os.path.dirname(output), '.makeMovie.' + time.strftime('%y-%m-%d_%H-%M-%S'))
   if os.path.isdir( tmpdir): shutil.rmtree( tmpdir)
if verbose: print 'TempDir = ' + tmpdir

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

# Cacher:
if draw169 or draw235: need_convert = True
if draw169 > 100: draw169 = 100
draw169_a = 0.01 * draw169
draw169_y = int((height - width*9/16) / 2)
draw169_h = height - draw169_y
if draw169_y < 1: draw169 = False
if draw235 > 100: draw235 = 100
draw235_a = 0.01 * draw235
draw235_y = int((height - width/2.35) / 2)
draw235_h = height - draw235_y
if draw235_y < 1: draw235 = False

imgCount = 0
# Pre composition:
cmd_precomp = []
name_precomp = []
# Generate header:
if need_convert:
   cmd = 'convert -size %(width)dx%(height)d -colorspace RGB xc:black' % vars()
   cmd += ' -fill "rgb(255,0,0)" -draw "rectangle  0,0 10,10"'
   cmd += ' -fill "rgb(0,255,0)" -draw "rectangle 10,0 20,10"'
   cmd += ' -fill "rgb(0,0,255)" -draw "rectangle 20,0 30,10"'
   rect_num = 10
   rect_w = width / (rect_num + 2)
   rect_c = 0
   rect_y1 = 30
   rect_y2 = 70
   rect_x1 = rect_w
   rect_x2 = rect_x1 + rect_w
   for r in range( 0, rect_num):
      cmd += ' -fill "rgb(%(rect_c)d,%(rect_c)d,%(rect_c)d)"' % vars()
      cmd += ' -draw "rectangle %(rect_x1)d,%(rect_y1)d %(rect_x2)d,%(rect_y2)d"' % vars()
      rect_x1 += rect_w
      rect_x2 += rect_w
      rect_c += 255 / (rect_num - 1)
   if font != '': cmd += ' -font %s' % font
   if project != '':
      cmd += ' -fill white -pointsize 30 -gravity northwest -annotate +10+100 "%s"' % project
   if datetimestring != '':
      cmd += ' -fill white -pointsize 30 -gravity north -annotate +10+100 "%s"' % datetimestring
   if company != '':
      cmd += ' -fill white -pointsize 30 -gravity northeast -annotate +10+100 "%s"' % company
   if shot != '':
      cmd += ' -fill white -pointsize 30 -gravity northwest -annotate +10+150 "%s"' % shot
   if shotversion != '':
      cmd += ' -fill white -pointsize 30 -gravity north -annotate +10+150 "%s"' % shotversion
   cmd += ' -fill white -pointsize 20 -gravity northeast -annotate +10+160 "frame range: 1-%d"' % len(allFiles)
   if artist != '':
      cmd += ' -fill white -pointsize 25 -gravity southwest -annotate +10+50 "%s"' % ('Artist: ' + artist)
   if activity != '':
      cmd += ' -fill white -pointsize 25 -gravity southeast -annotate +10+50 "%s"' % ('Activity: ' + activity)
   if drawfilename:
      cmd += ' -fill white -pointsize 30 -gravity southwest -annotate +10+10 "%s"' % os.path.basename(output)
   if comments != '':
      cmd += ' -fill white -pointsize 20 -gravity west -annotate +10+0 "%s"' % ('Comments: ' + comments)
   cmd += ' ' + os.path.join( tmpdir, tmpName) + '.%07d.' % imgCount + tmpFormat
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
      tmpLogo = os.path.join( tmpdir, tmpLogo)
      cmd = 'convert'
      cmd += ' ' + logopath
      cmd += ' -colorspace RGB'
      cmd += ' -resize "%s!"' % logosize
      cmd += ' -extent %(width)dx%(height)d' % vars()
      cmd += ' -affine 1,0,0,1,-%(logotx)d,-%(logoty)d -transform' % vars()
      cmd += ' ' + tmpLogo
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
      cmd = 'convert -size %(width)dx%(height)d -colorspace RGB xc:black' % vars()
      cmd += ' ' + os.path.join( inputdir, afile)
      cmd += ' -resize %(width)d' % vars()
      if gamma > 0: cmd += ' -gamma %.2f' % gamma
      if correction != '': cmd += correction
      cmd += ' -compose plus -gravity center -composite'
      if draw169:
         cmd += ' -fill "rgba(0,0,0,%(draw169_a)f)" -draw "rectangle 0,0,%(width)d,%(draw169_y)d"' % vars()
         cmd += ' -fill "rgba(0,0,0,%(draw169_a)f)" -draw "rectangle 0,%(draw169_h)d,%(width)d,%(height)d"' % vars()
      if draw235:
         cmd += ' -fill "rgba(0,0,0,%(draw235_a)f)" -draw "rectangle 0,0,%(width)d,%(draw235_y)d"' % vars()
         cmd += ' -fill "rgba(0,0,0,%(draw235_a)f)" -draw "rectangle 0,%(draw235_h)d,%(width)d,%(height)d"' % vars()
      if font != '': cmd += ' -font %s' % font
      if datetimestring != '':
         cmd += ' -fill white -pointsize 20 -gravity southwest -annotate +10+50 "%s"' % datetimestring
      if drawframe:
         digits = afile[ len(prefix) : len(afile) - len(suffix)]
         cmd += ' -fill white -pointsize 30 -gravity southeast -annotate +10+10 "%s"' % digits
      if project != '':
         cmd += ' -fill white -pointsize 30 -gravity northwest -annotate +10+10 "%s"' % project
      if shot != '' or shotversion != '':
         cmd += ' -fill white -pointsize 30 -gravity northeast -annotate +10+10 "%s"' % (shot + ' ' + shotversion)
      if drawfilename:
         cmd += ' -fill white -pointsize 30 -gravity southwest -annotate +10+10 "%s"' % os.path.basename(output)
      if need_logo:
         cmd += ' ' + tmpLogo
         cmd += ' -compose plus -composite'
      cmd += ' ' + os.path.join( tmpdir, tmpName) + '.%07d.' % imgCount + tmpFormat
      cmd_convert.append( cmd)
      name_convert.append( afile)
      imgCount += 1

# Encode commands:
auxargs = ''
if codec != 'ffmpeg':
   if not sys.platform.find('win') == 0: auxargs = '-threads 0'
   inputmask = os.path.join( inputdir, prefix+'%0'+str(digitsnum)+'d'+suffix)
   if len(cmd_convert): inputmask = os.path.join( tmpdir, tmpName+'.%07d.'+tmpFormat)
elif codec == 'mencoder':
   inputmask = os.path.join( inputdir, prefix+'*'+suffix)
   if len(cmd_convert): inputmask = os.path.join( tmpdir, tmpName+'.*.'+tmpFormat)
else:
   print 'Unknown encoder = "%s"' % encoder
   exit(1)
cmd_encode = cmd_encode.replace('@INPUT@',  inputmask)
cmd_encode = cmd_encode.replace('@FPS@',    str(fps))
cmd_encode = cmd_encode.replace('@OUTPUT@', output)
cmd_encode = cmd_encode.replace('@ARGS@',   auxargs)

# Print commands:
if debug:
   if len(cmd_precomp):
      print 'Precomp  first and last commands:'
      print
      print cmd_precomp[0]
      os.system( cmd_precomp[0])
      if len(cmd_precomp) > 1:
         print '...'
         print cmd_precomp[len(cmd_precomp)-1]
         os.system( cmd_precomp[len(cmd_precomp)-1])
      print
   if need_convert:
      print 'Convert first and last commands:'
      print
      print cmd_convert[0]
      os.system( cmd_convert[0])
      print '...'
      print cmd_convert[len(cmd_convert)-1]
      os.system( cmd_convert[len(cmd_convert)-1])
      print
   print 'Encode command:'
   print
   print cmd_encode
   os.system( cmd_encode)
   print
   exit(0)

# Construct Afanasy job:
if afanasy:
   af = __import__('af', globals(), locals(), [])
   j=af.Job( afjobname)
   if username != '': j.setUserName(username)

   if len(cmd_precomp):
      bp=af.Block( 'precomp', 'generic')
      j.blocks.append( bp)
      n = 0
      for cmd in cmd_precomp:
         t=af.Task( name_precomp[n])
         bp.tasks.append( t)
         t.setCommand( cmd)
         n += 1
      bp.setCapacity( afconvcap)

   if need_convert:
      bc=af.Block( 'convert', 'generic')
      j.blocks.append( bc)
      n = 0
      for cmd in cmd_convert:
         t=af.Task( name_convert[n])
         bc.tasks.append( t)
         t.setCommand( cmd)
         n += 1
      bc.setCapacity( afconvcap)
      if need_logo: bc.setDependMask('precomp')
      bc.setTasksMaxRunTime(11)

   be = af.Block( 'encode', 'generic')
   j.blocks.append( be)
   t = af.Task( output)
   be.tasks.append( t)
   t.setCommand( cmd_encode)
   be.setCapacity( afenccap)
   if need_convert:
      be.setDependMask('convert')
      j.setCmdPre( 'mkdir ' + os.path.abspath(tmpdir))
      j.setCmdPost('rm -R ' + os.path.abspath(tmpdir))

   if verbose: j.output(1)

# Commands execution:
if afanasy: j.send( verbose)
else:
   if len(cmd_precomp) or need_convert: os.mkdir(tmpdir, 0777)
   if len(cmd_precomp):
      n = 0
      print 'Precomositing...'
      for cmd in cmd_precomp:
         print name_precomp[n]
         os.system( cmd)
         n += 1
         print 'PROGRESS: %d' % (100.0 * n / len(cmd_precomp)) + '%'
         sys.stdout.flush()
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
   print
   print 'Done'
