#!/usr/bin/env python

import re, os, sys, subprocess

from optparse import OptionParser
Parser = OptionParser(usage="%prog [options] scan_folder result_folder\ntype \"%prog -h\" for help", version="%prog 1.  0")

Extensions = ['jpg','dpx','cin','exr','tga','tif','png']

BadFileCaracters = '. \\/:;`~!@#$%^&*()[]|,\'"{}'

Parser.add_option('-c', '--codec',        dest='codec',        type  ='string',     default='aphotojpg.ffmpeg', help='File with encode command line in last line')
Parser.add_option('-f', '--fps',          dest='fps',          type  ='int',        default=25,          help='Frames per second')
Parser.add_option('-r', '--resolution',   dest='resolution',   type  ='string',     default='768x576',   help='Movie resolution')
Parser.add_option('-e', '--extensions',   dest='extensions',   type  ='string',     default='',          help='Files extensions, comma searated')
Parser.add_option(      '--after',        dest='after',        type  ='int',        default=0,           help='Search for folder with time after')
Parser.add_option(      '--include',      dest='include',      type  ='string',     default='',          help='Include path pattern')
Parser.add_option(      '--exclude',      dest='exclude',      type  ='string',     default='',          help='Exclude path pattern')
Parser.add_option('-t', '--template',     dest='template',     type  ='string',     default='',          help='Specify frame template to use')
Parser.add_option('-g', '--gamma',        dest='gamma',        type  ='float',      default=-1.0,        help='Apply gamma correction')
Parser.add_option('-a', '--abspath',      dest='abspath',      action='store_true', default=False,       help='Prefix movies with images absolute path')
Parser.add_option('-A', '--afanasy',      dest='afanasy',      type  ='int',        default=0,           help='Send commands to Afanasy with specitied capacity')
Parser.add_option('-m', '--maxhosts',     dest='maxhosts',     type  ='int',        default=-1,          help='Afanasy maximum hosts parameter.')
Parser.add_option(      '--pause',        dest='pause',        action='store_true', default=False,       help='Start Afanasy job paused.')
Parser.add_option('-V', '--verbose',      dest='verbose',      action='store_true', default=False,       help='Verbose mode')
Parser.add_option('--aspect_in',          dest='aspect_in',    type  ='float',      default=-1.0,        help='Input image aspect, -1 = no changes')
Parser.add_option('--aspect_auto',        dest='aspect_auto',  type  ='float',      default=1.2,         help='Auto image aspect (2 if w/h <= aspect_auto), -1 = no changes')
Parser.add_option('-D', '--debug',        dest='debug',        action='store_true', default=False,       help='Debug mode (verbose mode, no commands execution)')
Parser.add_option(      '--test',         dest='test',         action='store_true', default=False,       help='Test mode, only show founded sequences.')

(Options, args) = Parser.parse_args()

if len(args) != 2: Parser.error('Not enough or too many arguments provided.')
Folder = args[0]
Output = args[1]
if not os.path.isdir(Folder): Parser.error('Scan folder "%s" does not exist.' % Folder)

if Options.debug: Options.verbose = True
if Options.test: Options.verbose = True
if Options.debug: print('DEBUG MODE:')

if Options.extensions != '':
   Extensions = []
   Extensions.extend(Options.extensions.split(','))

def isRightFile(afile):
   for ext in Extensions:
      if afile.find('.'+ext) == (len(afile) - len(ext) - 1):
         return True
   return False

REdigits = re.compile(r'\d+')
REinclude = None
REexclude = None
if Options.include != '': REinclude = re.compile( Options.include)
if Options.exclude != '': REexclude = re.compile( Options.exclude)

Command = os.path.join( os.path.dirname(sys.argv[0]), 'makemovie.py')
Command = '"%s" "%s"' % ( os.getenv('CGRU_PYTHONEXE','python'), Command)

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
         another_file_founded = False
         for other in filenames:
            if other == afile: continue
            if other[:pos] == afile[:pos] and other[pos+digitslen:] == afile[pos+digitslen:]:
               processedfiles.append(other)
               another_file_founded = True
         if not another_file_founded: continue
         pattern = afile[:pos] + '#'*digitslen + afile[pos+digitslen:]
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

if Options.afanasy != 0:
   af = __import__('af', globals(), locals(), [])
   job = af.Job('Scan ' + Folder)
   block = af.Block('movgen', 'movgen')
   block.setCapacity( Options.afanasy)
   job.blocks.append( block)
   if Options.maxhosts != -1: job.setMaxHosts( Options.maxhosts)
   if Options.pause: job.offline()
   job.setNeedOS('')

for dirpath, dirnames, filenames in os.walk( Folder):
   if REinclude is not None:
      includes = REinclude.findall( dirpath)
      if includes is None: continue
      if len(includes) == 0: continue
   if REexclude is not None:
      excludes = REexclude.findall( dirpath)
      if excludes is None: continue
      if len(excludes) != 0: continue
   if Options.after > 0:
      if os.path.getmtime( dirpath) < Options.after: continue
   patterns = getPatterns( filenames)
   for pattern in patterns:
      pattern = os.path.join( dirpath, pattern)
      if Options.verbose:
         print( pattern)
         sys.stdout.flush()
      if Options.test: continue

      outdir = os.path.join( dirpath, Output)

      movname = pattern
      if not Options.abspath:
         movname = movname.replace( Folder, '')
         movname = movname.strip('/\\')
      movname = genMovieName(movname)
      movname = os.path.join( outdir, movname)

      cmd = Command
      cmd += ' -r %s' % Options.resolution
      cmd += ' -f %d' % Options.fps
      cmd += ' -c %s' % Options.codec
      if Options.gamma > 0: cmd += ' -g %.2f' % Options.gamma
      if Options.aspect_in > 0: cmd += ' --aspect_in %f' % Options.aspect_in
      if Options.aspect_auto > 0: cmd += ' --aspect_auto %f' % Options.aspect_auto
      if Options.template != '': cmd += ' -t "%s"' % Options.template
      cmd += ' --createoutdir'
      cmd += ' "%s"' % pattern
      cmd += ' "%s"' % movname

      if Options.verbose: print( cmd)

      if not Options.debug:
         if Options.afanasy != 0:
            task = af.Task( os.path.basename(movname))
            task.setCommand( cmd)
            block.tasks.append( task)
         else:
            subprocess.Popen( cmd, shell=True).communicate()

if not Options.debug and not Options.test and Options.afanasy != 0: job.send()
