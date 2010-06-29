#!/usr/bin/env python

import sys, os, shutil

if len(sys.argv) <= 1:
   print 'Error: Files not specified.'
   print 'Usage: ' + sys.path.basename(sys.argv[0]) + ' files'
   sys.exit(0)

for arg in range( 1, len(sys.argv)):
   file = sys.argv[arg]
   if sys.platform.find('win') == 0:
      file = file.replace('/','\\')
      os.system('DEL /F /Q /S ' + file)
   else:
      file = file.replace('\\','/')
      os.system('rm -rvf ' + file)

sys.exit(0)
