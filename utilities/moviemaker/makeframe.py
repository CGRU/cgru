#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import re
import time
import subprocess

from optparse import OptionParser
Parser = OptionParser(usage="%prog [options] input [second_input] output\ntype \"%prog -h\" for help", version="%prog 1.  0")

Parser.add_option('-d', '--datetime',   dest='datetime',       type  ='string',     default='',          help='Draw date & time')
Parser.add_option('-c', '--company',    dest='company',        type  ='string',     default='',          help='Draw company')
Parser.add_option('-p', '--project',    dest='project',        type  ='string',     default='project',   help='Draw project')
Parser.add_option('-s', '--shot',       dest='shot',           type  ='string',     default='shot',      help='Draw shot')
Parser.add_option('-a', '--artist',     dest='artist',         type  ='string',     default='',          help='Draw artist')
Parser.add_option('-f', '--frame',      dest='frame',          type  ='string',     default='',          help='Draw frame')
Parser.add_option('-m', '--moviename',  dest='moviename',      type  ='string',     default='movie',     help='Draw final movie name')
Parser.add_option('--aspect_in',        dest='aspect_in',      type  ='float',      default=-1.0,        help='Input image aspect, "-1" = no changes')
Parser.add_option('--aspect_out',       dest='aspect_out',     type  ='float',      default=-1.0,        help='Output image aspect, "-1" = no changes')
Parser.add_option('--ver',              dest='shotversion',    type  ='string',     default='version',   help='Draw shot version')
Parser.add_option('--activity',         dest='activity',       type  ='string',     default='',          help='Draw activity')
Parser.add_option('--comments',         dest='comments',       type  ='string',     default='',          help='Draw comments')
Parser.add_option('--framerange',       dest='framerange',     type  ='string',     default='',          help='Draw frame range')
Parser.add_option('--filedate',         dest='filedate',       type  ='string',     default='',          help='Draw file date')

Parser.add_option('-t', '--template',   dest='template',       type  ='string',     default='',          help='Draw template to use')
Parser.add_option('-r', '--resolution', dest='resolution',     type  ='string',     default='720x576',   help='Format: 720x576')
Parser.add_option('-g', '--gamma',      dest='gamma',          type  ='float',      default=-1.0,        help='Apply gamma correction')
Parser.add_option('-q', '--quality',    dest='quality',        type  ='string',     default='',          help='Output image quality')
Parser.add_option('--noautocorr',       dest='noautocorr',     action='store_true', default=False,       help='Disable auto color correction for Cineon and EXR')
Parser.add_option('--correction',       dest='correction',     type  ='string',     default='',          help='Add custom color correction parameters')
Parser.add_option('--stereodub',        dest='stereodub',      action='store_true', default=False,       help='Stereo dublicate mode')
Parser.add_option('--drawcolorbars',    dest='drawcolorbars',  action='store_true', default=False,       help='Draw file name')
Parser.add_option('--cacher_aspect',    dest='cacher_aspect',  type  ='float',      default=1.85,        help='Cacher aspect')
Parser.add_option('--cacher_opacity',   dest='cacher_opacity', type  ='int',        default=0,           help='Cacher opacity')
Parser.add_option('--line_aspect',      dest='line_aspect',    type  ='float',      default=1.85,        help='Cacher line aspect')
Parser.add_option('--line_color',       dest='line_color',     type  ='string',     default='',          help='Cacher line opacity')
Parser.add_option('--draw169',          dest='draw169',        type  ='int',        default=0,           help='Draw 16:9 cacher opacity')
Parser.add_option('--draw235',          dest='draw235',        type  ='int',        default=0,           help='Draw 2.35 cacher opacity')
Parser.add_option('--line169',          dest='line169',        type  ='string',     default='',          help='Draw 16:9 line color: "255,255,0"')
Parser.add_option('--line235',          dest='line235',        type  ='string',     default='',          help='Draw 2.35 line color: "255,255,0"')
Parser.add_option('--font',             dest='font',           type  ='string',     default='',          help='Specify font)')
Parser.add_option('--logopath',         dest='logopath',       type  ='string',     default='',          help='Add a specified logo image, logo should be the same format')
Parser.add_option('-V', '--verbose',    dest='verbose',        action='store_true', default=False,       help='Verbose mode')
Parser.add_option('-D', '--debug',      dest='debug',          action='store_true', default=False,       help='Debug mode (verbose mode, no commands execution)')

(Options, args) = Parser.parse_args()

if len(args) < 1: Parser.error('Not enough arguments provided.')
if len(args) > 3: Parser.error('Too many arguments provided.')

FILEIN1        = ''
FILEIN2        = ''
FileOut        = args[0]
Stereo         = Options.stereodub
if len(args) == 2:
   FILEIN1     = args[0]
   FileOut     = args[1]
if len(args) == 3:
   FILEIN1     = args[0]
   FILEIN2     = args[1]
   FileOut     = args[2]
   Stereo      = True

MOVIENAME      = Options.moviename
DATETIME       = Options.datetime
COMPANY        = Options.company
PROJECT        = Options.project
SHOT           = Options.shot
VERSION        = Options.shotversion
ARTIST         = Options.artist
ACTIVITY       = Options.activity
COMMENTS       = Options.comments
FRAME          = Options.frame
FRAMERANGE     = Options.framerange
FILEDATE       = Options.filedate
FILEINBASE     = os.path.basename(FILEIN1)

# Parameters initialization:
if DATETIME == '': DATETIME = time.strftime('%y-%m-%d %H:%M')
if ARTIST == '': ARTIST = os.getenv('USER',os.getenv('USERNAME','user'))

Verbose = Options.verbose
if Options.debug: Verbose = True
if Options.debug: print('DEBUG MODE:')
if Verbose: print('VERBOSE MODE:')

# Read all commands from template:
cmdlines = []
template = Options.template
if template != '':
   template = os.path.join( os.path.join( os.path.dirname( sys.argv[0]),'templates'), template)
   if os.path.isfile(template):
      file = open( template)
      cmdlines = file.readlines()
      file.close()
   else:
      print('Error: template file "%s" not founded' % template)

# Resolution:
Width = 0
Height = 0
if Options.resolution != '':
   res = Options.resolution.split('x')
   if len(res) < 2: Parser.error('Invalid resolution specified.')
   Width = int(res[0])
   Height = int(res[1])
   if len(res) > 2 and Options.aspect_out < 0: Options.aspect_out = float(res[2])
   if Verbose: print('Output Resolution = %dx%dx%f' % ( Width, Height, Options.aspect_out))

# Cacher:
cacher169 = 16.0 / 9.0
cacher235 = 2.35
if Options.aspect_out > 0:
   Options.cacher_aspect = Options.cacher_aspect / Options.aspect_out
   Options.line_aspect = Options.line_aspect / Options.aspect_out
   cacher169 = cacher169 / Options.aspect_out
   cacher235 = cacher235 / Options.aspect_out
cacher_alpha = 0.01 * Options.cacher_opacity
if cacher_alpha > 1.0: cacher_alpha = 1.0
cacher_y = int((Height - float(Width)/Options.cacher_aspect) / 2)
cacher_h = Height - cacher_y
cacherline_y = int((Height - Width/Options.line_aspect) / 2)
cacherline_h = Height - cacherline_y
draw169_alpha = 0.01 * Options.draw169
if draw169_alpha > 1.0: draw169_alpha = 1.0
draw169_y = int((Height - Width/cacher169) / 2)
draw169_h = Height - draw169_y
draw235_alpha = 0.01 * Options.draw235
if draw235_alpha > 1.0: draw235_alpha = 1.0
draw235_y = int((Height - Width/cacher235) / 2)
draw235_h = Height - draw235_y

# Frame manipulate function:
def reformatAnnotate( infile, outfile):
   global Width,Height,Stereo
   global MOVIENAME,DATETIME,COMPANY,PROJECT,SHOT,VERSION,ARTIST,ACTIVITY,COMMENTS,FRAME,FRAMERANGE,FILEDATE,FILEINBASE
   global FILEIN
   FILEIN = os.path.basename( infile)
   # Input file indentify:
   if infile != '':
      # Check for an input file:
      if not os.path.isfile(infile): Parser.error('Input file "%s" does not exist.' % infile)
      # Get input file modification time:
      if FILEDATE == '': FILEDATE = time.strftime('%y/%m/%d', time.gmtime(os.stat(infile).st_mtime))
      # Get frame number if not specified:
      if FRAME == '':
         digits = re.findall(r'\d+', FILEIN)
         if digits is not None:
            if len(digits):
               FRAME = digits[-1]
               if Verbose: print('Frame = "%s"' % FRAME)

      # Get input file type:
      imgtype = ''
      lastdotpos = FILEIN.rfind('.')
      if lastdotpos > 1: imgtype = FILEIN[lastdotpos+1:].lower()
      if Verbose: print('Images type = "%s"' % imgtype)
      # Input file correction:
      correction = ''
      if not Options.noautocorr:
         corr_sRGB = '-set colorspace sRGB'
         corr_Log = '-set colorspace Log'
         # if sys.platform.find('win') == 0: corr_Log = '-set gamma 1.7 -set film-gamma 5.6 -set reference-black 95 -set reference-white 685 -colorspace srgb'
         if   imgtype == 'exr': correction = corr_sRGB
         elif imgtype == 'dpx': correction = corr_Log
         elif imgtype == 'cin': correction = corr_Log
      if Options.correction != '':
         if correction != '': correction += ' '
         correction += Options.correction
      # Get frame number if not specified:
      if FRAME == '':
         digits = re.findall(r'\d+', FILEIN)
         if digits is not None:
            if len(digits):
               FRAME = digits[-1]
               if Verbose: print('Frame = "%s"' % FRAME)

      cmd = 'convert "%s" +matte' % infile
      cmd += ' -resize %d' % Width
      if Options.aspect_in > 0 or Options.aspect_out > 0:
         scale = 100.0
         if Options.aspect_in  > 0: scale /= Options.aspect_in
         if Options.aspect_out > 0: scale *= Options.aspect_out
         cmd += ' -resize 100x%f%%' % scale
      cmd += ' -gravity center -background black -extent %(Width)dx%(Height)d +repage' % globals()
      if correction != '': cmd += ' %s' % correction
      if Options.gamma > 0: cmd += ' -gamma %.2f' % Options.gamma
      # Draw cacher:
      if Options.cacher_opacity > 0:
         cmd += ' -fill "rgba(0,0,0,%(cacher_alpha)f)" -draw "rectangle 0,0,%(Width)d,%(cacher_y)d"' % globals()
         cmd += ' -fill "rgba(0,0,0,%(cacher_alpha)f)" -draw "rectangle 0,%(cacher_h)d,%(Width)d,%(Height)d"' % globals()
      if Options.draw169 > 0:
         cmd += ' -fill "rgba(0,0,0,%(draw169_alpha)f)" -draw "rectangle 0,0,%(Width)d,%(draw169_y)d"' % globals()
         cmd += ' -fill "rgba(0,0,0,%(draw169_alpha)f)" -draw "rectangle 0,%(draw169_h)d,%(Width)d,%(Height)d"' % globals()
      if Options.draw235 > 0:
         cmd += ' -fill "rgba(0,0,0,%(draw235_alpha)f)" -draw "rectangle 0,0,%(Width)d,%(draw235_y)d"' % globals()
         cmd += ' -fill "rgba(0,0,0,%(draw235_alpha)f)" -draw "rectangle 0,%(draw235_h)d,%(Width)d,%(Height)d"' % globals()
      # Draw cacher lines:
      if Options.line_color != '':
         cmd += ' -fill "rgba('+Options.line_color+',1.0)" -draw "rectangle 0,%d,%d,%d"' % (cacherline_y-1,Width,cacherline_y)
         cmd += ' -fill "rgba('+Options.line_color+',1.0)" -draw "rectangle 0,%d,%d,%d"' % (cacherline_h,Width,cacherline_h+1)
      if Options.line169 != '':
         cmd += ' -fill "rgba('+Options.line169+',1.0)" -draw "rectangle 0,%d,%d,%d"' % (draw169_y-1,Width,draw169_y)
         cmd += ' -fill "rgba('+Options.line169+',1.0)" -draw "rectangle 0,%d,%d,%d"' % (draw169_h,Width,draw169_h+1)
      if Options.line235 != '':
         cmd += ' -fill "rgba('+Options.line235+',1.0)" -draw "rectangle 0,%d,%d,%d"' % (draw235_y-1,Width,draw235_y)
         cmd += ' -fill "rgba('+Options.line235+',1.0)" -draw "rectangle 0,%d,%d,%d"' % (draw235_h,Width,draw235_h+1)

   else:
      cmd = 'convert -size %(Width)dx%(Height)d -colorspace RGB xc:black -alpha Transparent -antialias' % globals()

   # Apply font:
   if Options.font != '': cmd += ' -font %s' % Options.font
   # Construct command from template:
   for line in cmdlines:
      line = line.strip()
      if len(line) < 1: continue
      if line[0] == '#': continue
      # Calculate relative size:
      while True:
         pos = line.find('@SIZE_')
         if pos == -1: break
         pos1 = line[pos:].find('_')
         if pos1 != -1:
            pos1 += pos
            pos2 = line[pos1:].find('/')
            if pos2 != -1:
               pos2 += pos1
               pos3 = line[pos2:].find('@')
               if pos3 != -1:
                  pos3 += pos2
                  if line[pos3-1] in 'XY':
                     if line[pos3-1] == 'Y': vertical = True
                     else: vertical = False
                     try:
                        size1 = int(line[pos1+1:pos2])
                        size2 = int(line[pos2+1:pos3-1])
                     except:
                        print( str(sys.exc_info()[1]))
                        size1 = -1
                        size2 = -1
                     if size1 != -1:
                        if vertical:
                           size = float(Height)
# Not needed, font size should not be changed with output aspect
#                           if Options.aspect_out > 0: size *= Options.aspect_out
                        else: size = float(Width)
                        size = str(int(size*size1/size2))
                        line = line[:pos]+size+line[pos3+1:]
                        continue
         print('Invalid size syntax in line:')
         print( line)
         print('Examples: "@SIZE_2/5X@" or "@SIZE_2/5Y@" means 2/5 part of X or Y dimension of a final image.')
         sys.exit(1)
      # Add line with replaced variables:
      cmd += ' ' + (line % globals())
   # Draw color bars:
   if Options.drawcolorbars:
      box_h = 10
# Output aspect not changing drawing now:
#      if Options.aspect_out > 0: box_h *= Options.aspect_out
      cmd += ' -fill "rgb(255,0,0)" -draw "rectangle  0,0 10,%d"' % box_h
      cmd += ' -fill "rgb(0,255,0)" -draw "rectangle 10,0 20,%d"' % box_h
      cmd += ' -fill "rgb(0,0,255)" -draw "rectangle 20,0 30,%d"' % box_h
      rect_num = 10
      rect_w = Width / (rect_num + 2)
      rect_c = 0
      rect_y1 = 30
      rect_y2 = 70
# Output aspect not changing drawing now:
#      if Options.aspect_out > 0:
#         rect_y1 *= Options.aspect_out
#         rect_y2 *= Options.aspect_out
      rect_x1 = rect_w
      rect_x2 = rect_x1 + rect_w
      for r in range( 0, rect_num):
         cmd += ' -fill "rgb(%(rect_c)d,%(rect_c)d,%(rect_c)d)"' % vars()
         cmd += ' -draw "rectangle %(rect_x1)d,%(rect_y1)d %(rect_x2)d,%(rect_y2)d"' % vars()
         rect_x1 += rect_w
         rect_x2 += rect_w
         rect_c += 255 / (rect_num - 1)
   # Add logo in path specified:
   if Options.logopath != '': cmd += ' "%s" -compose plus -gravity center -composite' % Options.logopath
   # Stereo:
   if Stereo: cmd += ' -resize "%dx%d!"' % ( Width/2, Height)
   # Set quality if specified:
   if Options.quality != '': cmd += ' -quality ' + Options.quality
   # Set/Reset common properties:
   cmd += ' -strip -density 72x72 -units PixelsPerInch -sampling-factor 1x1 -alpha Off'
   # Set output file name and launch:
   cmd += ' "%s"' % outfile
   if Verbose: print( cmd + '\n')
   if not Options.debug: os.system(cmd)


Annotate1 = FileOut
if Stereo:
   Annotate1 = os.path.join( os.path.dirname(FileOut), 'left.'  + os.path.basename(FileOut))
   Annotate2 = os.path.join( os.path.dirname(FileOut), 'right.' + os.path.basename(FileOut))

reformatAnnotate( FILEIN1, Annotate1)

if Stereo:
   if FILEIN2 != '':
      reformatAnnotate( FILEIN2, Annotate2)
   else:
      Annotate2 = Annotate1
   cmd = 'convert -size %(Width)dx%(Height)d -colorspace RGB xc:black -antialias' % globals()
   cmd += ' "%s" -compose over -gravity West -composite' % Annotate1
   cmd += ' "%s" -compose over -gravity East -composite' % Annotate2
   cmd += ' -alpha Off -strip -density 72x72 -units PixelsPerInch -sampling-factor 1x1'
   cmd += ' "%s"' % FileOut
   if Verbose: print( cmd + '\n')
   if not Options.debug: os.system(cmd)
