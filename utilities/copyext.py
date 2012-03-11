#!/usr/bin/env python

import os, shutil, sys

extensions = ['doc','pdf',
'blend','psd','hip','otl','mb','ma','max','xsi','scn','flw','fbx',
'nk','aep','comp','shk',
'bpj','ptp','3de',
'obj','3dl','chan','bclip',
'sh','bat','cmd','py','mel','ms']
#'dpx',

from optparse import OptionParser
parser = OptionParser(usage="%prog [options] sorce dest\ntype \"%prog -h\" for help", version="%prog 1.  0")

parser.add_option('-c', '--copy',      dest='copy',    action='store_true', default=False, help='Perform a copy operations.')
parser.add_option('-V', '--verbose',   dest='verbose', action='store_true', default=False, help='Verbose mode.')

(options, args) = parser.parse_args()

if len( args) < 2: parser.error('Not enough arguments provided.')
Source = args[0]
Dest = args[1]
Verbose = options.verbose
if not options.copy: Verbose = True
if os.path.basename( Source) != os.path.basename( Dest):
   Dest = os.path.join( Dest, os.path.basename( Source))

files = []
for dirpath, dirnames, filenames in os.walk( Source, False, None):
   if Verbose: print dirpath
   for filename in filenames:
      dot = filename.rfind('.')
      if dot < 1: continue
      for ext in extensions:
         if filename[dot+1:] != ext: continue
         src = os.path.join( dirpath, filename)
         files.append(src)
         if Verbose: print src

sizes = []
for src in files:
   statinfo = os.stat(src)
   sizes.append(statinfo.st_size)

size_total = 0
for size in sizes: size_total += size
print 'Total size = ' + str(size_total/1024/1024) + ' Mb'

if not options.copy: sys.exit(0)

counter = 0
size_copy = 0
dest_folder_old = ''
for src in files:
   dest_folder = os.path.join( Dest, os.path.dirname(src)[len(Source)+1:])
   if dest_folder != dest_folder_old:
      if not os.path.isdir( dest_folder):
         os.makedirs( dest_folder)
      dest_folder_old = dest_folder
   dst = os.path.join( dest_folder, os.path.basename(src))
   if not os.path.isfile( dst):
      print src + ' -> ' + dst
      try:
         shutil.copy( src, dst)
      except:
         print 'Error: ' + str(sys.exc_info()[1])
   size_copy += sizes[counter]
   print str(100 * size_copy / size_total) + ' %'
   counter += 1

print 'Total size = ' + str(size_total/1024/1024) + ' Mb'
