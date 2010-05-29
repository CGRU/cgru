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
   if os.path.isdir( tmpdir): shutil.rmtree( tmpdir)
   exit(0)

signal.signal(signal.SIGTERM, rmdir)
signal.signal(signal.SIGABRT, rmdir)
signal.signal(signal.SIGQUIT, rmdir)
signal.signal(signal.SIGINT,  rmdir)

from optparse import OptionParser
parser = OptionParser(usage="%prog [options]\ntype \"%prog -h\" for help", version="%prog 1.0")
parser.add_option('-f', '--fps',        dest='fps',         type  ='int',        default=25,          help='Frames per second')
parser.add_option('-c', '--codec',      dest='codec',       type  ='string',     default='mov',       help='Codec: mov,mpeg')
parser.add_option('-r', '--resolution', dest='resolution',  type  ='string',     default='',          help='Format: 768x576, if empty images format used')
parser.add_option('-i', '--inpattern',  dest='inpattern',   type  ='string',     default='',          help='Input files pattern: img.####.jpg')
parser.add_option('-o', '--output',     dest='output',      type  ='string',     default='',          help='Output filename, if not specified, pattern will be used')
parser.add_option('-g', '--gamma',      dest='gamma',       type  ='float',      default=-1.0,        help='Apply gamma correction')
parser.add_option('--project',          dest='project',     type  ='string',     default='',          help='Draw project')
parser.add_option('--shot',             dest='shot',        type  ='string',     default='',          help='Draw shot')
parser.add_option('--shotversion',      dest='shotversion', type  ='string',     default='',          help='Draw shot version')
parser.add_option('--artist',           dest='artist',      type  ='string',     default='',          help='Draw artist')
parser.add_option('--activity',         dest='activity',    type  ='string',     default='',          help='Draw activity')
parser.add_option('--annotate',         dest='annotate',    type  ='string',     default='',          help='Draw annotation if specified')
parser.add_option('--font',             dest='font',        type  ='string',     default='',          help='Specify font)')
parser.add_option('--logopath',         dest='logopath',    type  ='string',     default='',          help='Add a specified image')
parser.add_option('--logosize',         dest='logosize',    type  ='string',     default='200x100',   help='Logotype size')
parser.add_option('--drawdate',         dest='drawdate',    action='store_true', default=False,       help='Draw date')
parser.add_option('--drawframe',        dest='drawframe',   action='store_true', default=False,       help='Draw frame')
parser.add_option('--drawfilename',     dest='drawfilename',action='store_true', default=False,       help='Draw file name')
parser.add_option('--draw169',          dest='draw169',     type  ='int',        default=0,           help='Draw 16:9 cacher opacity')
parser.add_option('--draw235',          dest='draw235',     type  ='int',        default=0,           help='Draw 2.35 cacher opacity')
parser.add_option('-d', '--datesuffix', dest='datesuffix',  action='store_true', default=False,       help='Add date suffix to name')
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
drawframe      = options.drawframe
drawfilename   = options.drawfilename
draw169        = options.draw169
draw235        = options.draw235
font           = options.font
logopath       = options.logopath
logosize       = options.logosize
project        = options.project
shot           = options.shot
shotversion    = options.shotversion
artist         = options.artist
activity       = options.activity
annotate       = options.annotate
datesuffix     = options.datesuffix
gamma          = options.gamma

verbose     = options.verbose
debug       = options.debug

afanasy     = options.afanasy
afconvcap   = options.afconvcap
afenccap    = options.afenccap
username    = options.username

tmpdir      = options.tmpdir

# Definitions:
tmpName = 'img'
tmpFormat = 'jpg'
tmpLogo = 'logo.png'

need_convert = False
need_logo = False

# Check required parameters:
if inpattern == '': parser.error('Input files not specified.')

# Parameters initialization:
if debug: verbose = True
if verbose: print 'VERBOSE MODE:'
if debug: print 'DEBUG MODE:'
if drawdate or drawframe: need_convert = True
if project     != '': need_convert = True
if shot        != '': need_convert = True
if shotversion != '': need_convert = True
if artist      != '': need_convert = True
if activity    != '': need_convert = True
if annotate    != '': need_convert = True

# Input directory:
inputdir = os.path.dirname( inpattern)
if verbose: print 'InputDir = ' + inputdir
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
if datesuffix: output += '.`date +%y-%m-%d_%H-%M`'
if codec == 'mov': output += '.mov'
elif codec == 'mpeg': output += '.avi'
else:
   print 'Codec "%s" is not supported' % codec
   exit(1)
if verbose: print 'Output = ' + output
afjobname += ' %s' % codec

# Temporary directory:
if tmpdir == '': tmpdir = os.path.join( os.path.dirname(output), '.makeMovie.' + time.strftime('%y-%m-%d_%H-%M-%S'))
if os.path.isdir( tmpdir): shutil.rmtree( tmpdir)
if verbose: print 'TempDir = ' + tmpdir

# Resolution:
width = 0
height = 0
if resolution != '':
   need_convert = True
   pos = resolution.find('x')
   width = int(resolution[ 0 : pos ])
   height = int(resolution[ pos + 1 : len(resolution) ])
   if pos <= 0:
      print 'Invalid resolution specified.'
      exit(1)
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

# Generate reformat logo command:
cmd_convertlogo = ''
if logopath != '':
   if need_convert:
      need_logo = True
      tmpLogo = os.path.join( tmpdir, tmpLogo)
      cmd_convertlogo = 'convert -size %(width)dx%(height)d xc:black' % vars()
      cmd_convertlogo += ' -compose plus -gravity southeast -composite'
      cmd_convertlogo += ' ' + logopath
#      cmd_convertlogo += ' -gravity southeast -background black'
      cmd_convertlogo += ' -resize ' + logosize
#      cmd_convertlogo += ' -extent %(width)dx%(height)d' % vars()
      cmd_convertlogo += ' ' + tmpLogo
   else:
      print 'Can\'t add logo if output resolution is not specified.'
      exit(1)

# Generate convert commands lists:
cmd_convert = []
name_convert = []
imgCount = 0
if need_convert:
   for afile in allFiles:
      cmd = 'convert -size %(width)dx%(height)d xc:black' % vars()
      if need_logo: cmd += ' ' + tmpLogo
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
      fontneeded = False
      if drawdate:
         cmd += ' -fill white -pointsize 20 -gravity southwest -annotate +10+50 `date +%y-%m-%d_%H-%M`'
         fontneeded = True
      if drawframe:
         digits = afile[ len(prefix) : len(afile) - len(suffix)]
         cmd += ' -fill white -pointsize 30 -gravity southeast -annotate +10+10 "%s"' % digits
         fontneeded = True
      if project != '':
         cmd += ' -fill white -pointsize 30 -gravity northwest -annotate +10+10 "%s"' % project
         fontneeded = True
      if shot != '' or version != '':
         cmd += ' -fill white -pointsize 30 -gravity northeast -annotate +10+10 "%s"' % (shot + ' ' + shotversion)
         fontneeded = True
      if artist != '':
         cmd += ' -fill white -pointsize 25 -gravity northwest -annotate +10+50 "%s"' % artist
         fontneeded = True
      if activity != '':
         cmd += ' -fill white -pointsize 25 -gravity northeast -annotate +10+50 "%s"' % activity
         fontneeded = True
      if annotate != '':
         cmd += ' -fill white -pointsize 15 -gravity north -annotate +0+10 "%s"' % annotate
         fontneeded = True
      if drawfilename:
         cmd += ' -fill white -pointsize 30 -gravity southwest -annotate +10+10 ' + os.path.basename(output)
         fontneeded = True
      if fontneeded and font != '': cmd += ' -font %s' % font
#      if need_logo: cmd += ' -compose plus -composite'
      cmd += ' ' + os.path.join( tmpdir, tmpName) + '.%07d.' % imgCount + tmpFormat
      cmd_convert.append( cmd)
      name_convert.append( afile)
      imgCount += 1

# Encode commands:
cmd_encode = ''
if codec == 'mov':
   inputmask = os.path.join( inputdir, prefix+'%0'+str(digitsnum)+'d'+suffix)
   if len(cmd_convert): inputmask = os.path.join( tmpdir, tmpName+'.%07d.'+tmpFormat)
   cmd = 'ffmpeg'
   cmd += ' -y -r ' + str(fps) + ' -i '
   cmd += inputmask
   cmd += ' -vcodec mjpeg -qscale 1 '
   cmd += output
   cmd = cmd + ' || echo Done'
   cmd_encode = cmd
elif codec == 'mpeg':
   inputmask = os.path.join( inputdir, prefix+'*'+suffix)
   if len(cmd_convert): inputmask = os.path.join( tmpdir, tmpName+'.*.'+tmpFormat)
   cmd = 'mencoder'
   cmd += ' "mf://%s"' % inputmask
   cmd += ' -mf fps=' + str(fps) + ' -o ' + output + ' -ovc xvid -xvidencopts fixed_quant=1'
   cmd_encode = cmd
else:
   print 'Codec "%s" is not supported' % codec
   exit(1)

# Print commands:
if verbose:
   if need_logo:
      print 'Reformating logotype command:'
      print
      print cmd_convertlogo
      print
   if need_convert:
      print 'Convert first and last commands:'
      print
      print cmd_convert[0]
      print '...'
      print cmd_convert[len(cmd_convert)-1]
      print
   print 'Encode command:'
   print
   print cmd_encode
   print

# Construct Afanasy job:
if afanasy:
   af = __import__('af', globals(), locals(), [])
   j=af.Job( afjobname)
   if username != '': j.setUserName(username)

   if need_logo:
      bl=af.Block( 'logo', 'generic')
      j.blocks.append( bl)
      t=af.Task( logopath)
      bl.tasks.append( t)
      t.setCommand( cmd_convertlogo)
      bl.setCapacity( afconvcap)
#      bl.setHostsMaskExclude('l.*')

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
#      bc.setHostsMask('r.*')
      if need_logo: bc.setDependMask('logo')
      bc.setTasksMaxRunTime(11)

   be = af.Block( 'encode', 'generic')
   j.blocks.append( be)
   t = af.Task( output)
   be.tasks.append( t)
   t.setCommand( cmd_encode)
   be.setCapacity( afenccap)
   if need_convert:
      if need_logo: be.setDependMask('convert|logo')
      else: be.setDependMask('convert')
      j.setCmdPre( 'mkdir ' + os.path.abspath(tmpdir))
      j.setCmdPost('rm -R ' + os.path.abspath(tmpdir))

   if verbose: j.output(1)

# Commands execution:
if debug == False:
   if afanasy: j.send( verbose)
   else:
      if need_logo or need_convert: os.mkdir(tmpdir, 0777)
      if need_logo:
         print 'Reformatting logo...'
         os.system( cmd_convertlogo)
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
      if os.path.isdir( tmpdir): shutil.rmtree( tmpdir)
      print
      print 'Done'
