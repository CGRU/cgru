#!/usr/bin/env python

import re, os, sys

from optparse import OptionParser
parser = OptionParser(usage="%prog [options] scan_folder result_folder\ntype \"%prog -h\" for help", version="%prog 1.  0")

Extensions = ['jpg','dpx','cin','exr','tga','tif','png']

BadFileCaracters = '. \\/:;`~!@#$%^&*()[]|,\'"{}'

parser.add_option('-c', '--codec',        dest='codec',        type  ='string',     default='aphotojpg.ffmpeg', help='File with encode command line in last line')
parser.add_option('-f', '--fps',          dest='fps',          type  ='int',        default=25,          help='Frames per second')
parser.add_option('-r', '--resolution',   dest='resolution',   type  ='string',     default='768x576',   help='Movie resolution')
parser.add_option('-e', '--extensions',   dest='extensions',   type  ='string',     default='',          help='Files extensions, comma searated')
parser.add_option('-t', '--template',     dest='template',     type  ='string',     default='',          help='Specify frame template to use')
parser.add_option('-g', '--gamma',        dest='gamma',        type  ='float',      default=-1.0,        help='Apply gamma correction')
parser.add_option('-a', '--abspath',      dest='abspath',      action='store_true', default=False,       help='Prefix movies with images absolute path')
parser.add_option('-A', '--afanasy',      dest='afanasy',      type  ='int',        default=0,           help='Send commands to Afanasy with specitied capacity')
parser.add_option('-m', '--maxhosts',     dest='maxhosts',     type  ='int',        default=-1,          help='Afanasy maximum hosts parameter.')
parser.add_option('-V', '--verbose',      dest='verbose',      action='store_true', default=False,       help='Verbose mode')
parser.add_option('-D', '--debug',        dest='debug',        action='store_true', default=False,       help='Debug mode (verbose mode, no commands execution)')

(options, args) = parser.parse_args()

if len(args) != 2: parser.error('Not enough or too many arguments provided.')
Folder = args[0]
Output = args[1]
if not os.path.isdir(Folder): parser.error('Scan folder "%s" does not exist.' % Folder)
if not os.path.isdir(Output): parser.error('Scan folder "%s" does not exist.' % Output)

Verbose = options.verbose
if options.debug: Verbose = True
if options.debug: print 'DEBUG MODE:'
if Verbose: print 'VERBOSE MODE:'

if options.extensions != '':
   Extensions = []
   Extensions.extend(options.extensions.split(','))

def isRightFile(afile):
   for ext in Extensions:
      if afile.find('.'+ext) == (len(afile) - len(ext) - 1):
         return True
   return False

REdigits = re.compile(r'\d+')

Command = os.path.join( os.path.dirname(sys.argv[0]), 'mavishky.py')
Command = 'python ' + Command

def getPatterns(filenames):
   patterns = []
   processedfiles = []
   for afile in filenames:
      if afile in processedfiles: continue
      if isRightFile(afile):
         processedfiles.append(afile)
         digits = REdigits.findall(afile)
         if digits is None: continue
         if len(digits) == 0: continue
         digits = digits[-1]
         digitslen = len(digits)
         pos = afile.rfind(digits)
         for other in filenames:
            if other == afile: continue
            if other[:pos] == afile[:pos] and other[pos+digitslen:] == afile[pos+digitslen:]:
               processedfiles.append(other)
         pattern = afile[:pos]
         for i in range(0,digitslen): pattern += '#'
         pattern += afile[pos+digitslen:]
         patterns.append(pattern)
   return patterns

def genMovieName( path):
   lastdotpos = path.rfind('.')
   if lastdotpos > 0: path = path[:lastdotpos-1]
   for char in BadFileCaracters:
      path = path.replace( char, '_')
   path = path.strip('_')
   while path.find('__') != -1: path = path.replace('__', '_')
   return path

if options.afanasy != 0:
   af = __import__('af', globals(), locals(), [])
   job = af.Job('Scan ' + Folder)
   block = af.Block('movgen', 'movgen')
   block.setCapacity( options.afanasy)
   job.blocks.append( block)
   if options.maxhosts != -1: job.setMaxHosts( options.maxhosts)
   job.setNeedOS('')

for dirpath, dirnames, filenames in os.walk( Folder):
   patterns = getPatterns(filenames)
   for pattern in patterns:
      pattern = os.path.join( dirpath, pattern)
      if Verbose: print pattern

      movname = pattern
      if not options.abspath:
         movname = movname.replace( Folder, '')
         movname = movname.strip('/\\')
      movname = genMovieName(movname)
      movname = os.path.join( Output, movname)

      cmd = Command
      cmd += ' -r %s' % options.resolution
      cmd += ' -f %d' % options.fps
      cmd += ' -c %s' % options.codec
      if options.gamma > 0: cmd += ' -g %.2f' % options.gamma
      if options.template != '': cmd += ' -t "%s"' % options.template
      cmd += ' "%s"' % pattern
      cmd += ' "%s"' % movname

      if Verbose: print cmd

      if not options.debug:
         if options.afanasy != 0:
            task = af.Task( os.path.basename(movname))
            task.setCommand( cmd)
            block.tasks.append( task)
         else: os.system(cmd)

if not options.debug and options.afanasy != 0: job.send()
