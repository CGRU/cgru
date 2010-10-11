#!/usr/bin/env python

import os
import sys
import re
import time
import subprocess

from optparse import OptionParser
parser = OptionParser(usage="%prog [options] input [second_input] output\ntype \"%prog -h\" for help", version="%prog 1.  0")

parser.add_option('-d', '--datetime',   dest='datetime',       type  ='string',     default='',          help='Draw date & time')
parser.add_option('-c', '--company',    dest='company',        type  ='string',     default='company',   help='Draw company')
parser.add_option('-p', '--project',    dest='project',        type  ='string',     default='project',   help='Draw project')
parser.add_option('-s', '--shot',       dest='shot',           type  ='string',     default='shot',      help='Draw shot')
parser.add_option('-a', '--artist',     dest='artist',         type  ='string',     default='',          help='Draw artist')
parser.add_option('-f', '--frame',      dest='frame',          type  ='string',     default='',          help='Draw frame')
parser.add_option('-m', '--moviename',  dest='moviename',      type  ='string',     default='movie',     help='Draw final movie name')
parser.add_option('--ver',              dest='shotversion',    type  ='string',     default='ver',       help='Draw shot version')
parser.add_option('--activity',         dest='activity',       type  ='string',     default='',          help='Draw activity')
parser.add_option('--comments',         dest='comments',       type  ='string',     default='',          help='Draw comments')
parser.add_option('--framerange',       dest='framerange',     type  ='string',     default='',          help='Draw frame range')
parser.add_option('--filedate',         dest='filedate',       type  ='string',     default='',          help='Draw file date')

parser.add_option('-t', '--template',   dest='template',       type  ='string',     default='',          help='Draw template to use')
parser.add_option('-r', '--resolution', dest='resolution',     type  ='string',     default='720x576',   help='Format: 720x576')
parser.add_option('-g', '--gamma',      dest='gamma',          type  ='float',      default=-1.0,        help='Apply gamma correction')
parser.add_option('-q', '--quality',    dest='quality',        type  ='string',     default='',          help='Output image quality')
parser.add_option('--stereo',           dest='stereo',         action='store_true', default=False,       help='Stereo mode')
parser.add_option('--drawcolorbars',    dest='drawcolorbars',  action='store_true', default=False,       help='Draw file name')
parser.add_option('--draw169',          dest='draw169',        type  ='int',        default=0,           help='Draw 16:9 cacher opacity')
parser.add_option('--draw235',          dest='draw235',        type  ='int',        default=0,           help='Draw 2.35 cacher opacity')
parser.add_option('--font',             dest='font',           type  ='string',     default='',          help='Specify font)')
parser.add_option('--logopath',         dest='logopath',       type  ='string',     default='',          help='Add a specified image')
parser.add_option('-V', '--verbose',    dest='verbose',        action='store_true', default=False,       help='Verbose mode')
parser.add_option('-D', '--debug',      dest='debug',          action='store_true', default=False,       help='Debug mode (verbose mode, no commands execution)')

(options, args) = parser.parse_args()

if len(args) < 1: parser.error('Not enough arguments provided.')
if len(args) > 3: parser.error('Too many arguments provided.')

FILEIN1        = ''
FILEIN2        = ''
FileOut        = args[0]
Stereo         = options.stereo
if len(args) == 2:
   FILEIN1     = args[0]
   FileOut     = args[1]
if len(args) == 3:
   FILEIN1     = args[0]
   FILEIN2     = args[1]
   FileOut     = args[2]
   Stereo      = True
MOVIENAME      = options.moviename
DATETIME       = options.datetime
COMPANY        = options.company
PROJECT        = options.project
SHOT           = options.shot
VERSION        = options.shotversion
ARTIST         = options.artist
ACTIVITY       = options.activity
COMMENTS       = options.comments
FRAME          = options.frame
FRAMERANGE     = options.framerange
FILEDATE       = options.filedate
FILEINBASE     = os.path.basename(FILEIN1)

# Parameters initialization:
if DATETIME == '': DATETIME = time.strftime('%y-%m-%d %H:%M')
if ARTIST == '': ARTIST = os.getenv('USER',os.getenv('USERNAME','user'))

Verbose = options.verbose
if options.debug: Verbose = True
if options.debug: print 'DEBUG MODE:'
if Verbose: print 'VERBOSE MODE:'

# Read all commands from template:
cmdlines = []
template = options.template
if template != '':
   template = os.path.join( os.path.join( os.path.dirname( sys.argv[0]),'templates'), template)
   if os.path.isfile(template):
      file = open( template)
      cmdlines = file.readlines()
      file.close()
   else:
      print 'Error: template file "%s" not founded' % template

# Resolution:
Width = 0
Height = 0
if options.resolution != '':
   pos = options.resolution.find('x')
   if pos <= 0: parser.error('Invalid resolution specified.')
   Width = int(options.resolution[: pos ])
   Height = int(options.resolution[ pos + 1 :])
   if Verbose: print 'Output resolution = %(Width)d x %(Height)d' % vars()

# Cacher:
draw169_a = 0.01 * options.draw169
if draw169_a > 1.0: draw169_a =1.0
draw169_y = int((Height - Width*9/16) / 2)
draw169_h = Height - draw169_y
draw235_a = 0.01 * options.draw235
if draw235_a > 1.0: draw235_a =1.0
draw235_y = int((Height - Width/2.35) / 2)
draw235_h = Height - draw235_y

Counter = 0

def paintAnnotate( FILEIN):
   global Width,Height,Stereo,FileOut
   global MOVIENAME,DATETIME,COMPANY,PROJECT,SHOT,VERSION,ARTIST,ACTIVITY,COMMENTS,FRAME,FRAMERANGE,FILEDATE,FILEINBASE
   global Counter
   Counter += 1
   # Input file indentify:
   if FILEIN != '':
      # Check for an input file:
      if not os.path.isfile(FILEIN): parser.error('Input file "%s" does not exist.' % FILEIN1)
      # Get input file modification time:
      if FILEDATE == '': FILEDATE = time.strftime('%y/%m/%d', time.gmtime(os.stat(FILEIN).st_mtime))
      # Get frame number if not specified:
      if FRAME == '':
         digits = re.findall(r'\d+', FILEIN)
         if digits is not None:
            if len(digits):
               FRAME = digits[-1]
               if Verbose: print 'Frame = "%s"' % FRAME
   cmd = 'convert -size %(Width)dx%(Height)d -colorspace RGB xc:black -alpha Transparent -antialias' % globals()
   # Draw color bars:
   if options.drawcolorbars:
      cmd += ' -fill "rgb(255,0,0)" -draw "rectangle  0,0 10,10"'
      cmd += ' -fill "rgb(0,255,0)" -draw "rectangle 10,0 20,10"'
      cmd += ' -fill "rgb(0,0,255)" -draw "rectangle 20,0 30,10"'
      rect_num = 10
      rect_w = Width / (rect_num + 2)
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
   # Apply font:
   if options.font != '': cmd += ' -font %s' % options.font
   # Construct command from template:
   for line in cmdlines: cmd += ' ' + line.strip() % globals()
   # Add logo in path specified:
   if options.logopath != '': cmd += ' "%s" -compose plus -composite' % options.logopath
   # Stereo:
   if Stereo: cmd += ' -resize "%dx%d!"' % ( Width/2, Height)
   # Set quality if specified:
   if options.quality != '': cmd += ' -quality ' + options.quality
   # Set/Reset common properties:
   cmd += ' -strip -density 72x72 -units PixelsPerInch -sampling-factor 1x1'
   # Set output file name and launch:
   outname = os.path.join( os.path.dirname(FileOut), 'ann' + str(Counter) + '.' + os.path.basename(FileOut))
   cmd += ' "%s"' % outname
   if Verbose:
      print cmd
      print
   if not options.debug: os.system(cmd)
   return outname

def reformatImage( FILEIN):
   global Width,Height,FileOut
   global MOVIENAME,DATETIME,COMPANY,PROJECT,SHOT,VERSION,ARTIST,ACTIVITY,COMMENTS,FRAME,FRAMERANGE,FILEDATE,FILEINBASE
   global Counter
   Counter += 1
   # Check for an input file:
   if not os.path.isfile(FILEIN): parser.error('Input file "%s" does not exist.' % FILEIN1)
   # Get input file modification time:
   if FILEDATE == '': FILEDATE = time.strftime('%y/%m/%d', time.gmtime(os.stat(FILEIN).st_mtime))
   # Get input file type:
   imgtype = ''
   lastdotpos = FILEIN.rfind('.')
   if lastdotpos > 1: imgtype = FILEIN[lastdotpos+1:]
   if Verbose: print 'Images type = "%s"' % imgtype
   # Input file correction:
   correction = ''
   corr_sRGB = ' -gamma 2.2'
   corr_Log = ' -level 9%,67%,.6'
   if sys.platform.find('win') == 0:
      corr_Log = ' -set gamma 1.7 -set film-gamma 5.6 -set reference-black 95 -set reference-white 685 -colorspace srgb'
   if   imgtype == 'exr': correction = corr_sRGB
   elif imgtype == 'dpx': correction = corr_Log
   elif imgtype == 'cin': correction = corr_Log
   # Get frame number if not specified:
   if FRAME == '':
      digits = re.findall(r'\d+', FILEIN)
      if digits is not None:
         if len(digits):
            FRAME = digits[-1]
            if Verbose: print 'Frame = "%s"' % FRAME

   cmd = 'convert "%(FILEIN)s" +matte' % vars()
   cmd += ' -resize %(Width)d -gravity center -background black -extent %(Width)dx%(Height)d +repage' % globals()
   if correction != '': cmd += correction
   if options.gamma > 0: cmd += ' -gamma %.2f' % options.gamma
   # Draw cacher:
   if options.draw169 > 0:
      cmd += ' -fill "rgba(0,0,0,%(draw169_a)f)" -draw "rectangle 0,0,%(Width)d,%(draw169_y)d"' % globals()
      cmd += ' -fill "rgba(0,0,0,%(draw169_a)f)" -draw "rectangle 0,%(draw169_h)d,%(Width)d,%(Height)d"' % globals()
   if options.draw235 > 0:
      cmd += ' -fill "rgba(0,0,0,%(draw235_a)f)" -draw "rectangle 0,0,%(Width)d,%(draw235_y)d"' % globals()
      cmd += ' -fill "rgba(0,0,0,%(draw235_a)f)" -draw "rectangle 0,%(draw235_h)d,%(Width)d,%(Height)d"' % globals()
   # Stereo:
   if Stereo: cmd += ' -resize "%dx%d!"' % ( Width/2, Height)
   # Set quality if specified:
   if options.quality != '': cmd += ' -quality ' + options.quality
   # Set/Reset common properties:
   cmd += ' -strip -density 72x72 -units PixelsPerInch -sampling-factor 1x1'
   # Set output file name and launch:
   outname = os.path.join( os.path.dirname(FileOut), 'ref' + str(Counter) + '.' + os.path.basename(FileOut))
   cmd += ' "%s"' % outname
   if Verbose:
      print cmd
      print
   if not options.debug: os.system(cmd)
   return outname

Annotate1 = ''
Annotate2 = ''
Reformat1 = ''
Reformat2 = ''

Annotate1 = paintAnnotate( FILEIN1)
if FILEIN1 != '': Reformat1 = reformatImage( FILEIN1)

if Stereo:
   if FILEIN2 != '':
      Annotate2 = paintAnnotate( FILEIN2)
      Reformat2 = reformatImage( FILEIN2)
   else:
      Annotate2 = Annotate1
      Reformat2 = Reformat1

cmd = 'convert -size %(Width)dx%(Height)d -colorspace RGB xc:black -antialias' % globals()
if Reformat1 != '': cmd += ' "%s" -compose over -gravity West -composite' % Reformat1
if Reformat2 != '': cmd += ' "%s" -compose over -gravity East -composite' % Reformat2
if Annotate1 != '': cmd += ' "%s" -compose over -gravity West -composite' % Annotate1
if Annotate2 != '': cmd += ' "%s" -compose over -gravity East -composite' % Annotate2

# Set/Reset common properties:
cmd += ' -alpha Off -strip -density 72x72 -units PixelsPerInch -sampling-factor 1x1'
# Set output file name and launch:
cmd += ' "%s"' % FileOut
if Verbose:
   print cmd
   print
if not options.debug: os.system(cmd)
