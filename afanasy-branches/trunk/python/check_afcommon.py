#!/usr/bin/env python

import os
import sys

import afcommon

if len(sys.argv) < 2:
   print('\nUsage: To check common functions class launch:')
   print( os.path.basename(sys.argv[0]) + ' [func] [args...]\n')
   sys.exit(1)

if sys.argv[1] == 'pat2':
   if len(sys.argv) < 4:
      print('Usage: ' + sys.argv[0] + ' ' + sys.argv[1] + ' str1 str2')
   else:
      print( afcommon.patternFromPaths( sys.argv[2], sys.argv[3]))
elif sys.argv[1] == 'patc':
   if len(sys.argv) < 3:
      print( 'Usage: ' + sys.argv[0] + ' ' + sys.argv[1] + ' str')
   else:
      print( afcommon.patternFromStdC( sys.argv[2], True))
elif sys.argv[1] == 'patd':
   if len(sys.argv) < 3:
      print( 'Usage: ' + sys.argv[0] + ' ' + sys.argv[1] + ' str')
   else:
      print( afcommon.patternFromDigits( sys.argv[2], True))
else:
   print( 'Unknown command: "%s"' % sys.argv[1])