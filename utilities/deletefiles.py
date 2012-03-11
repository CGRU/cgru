#!/usr/bin/env python

import os, sys
import shutil

from optparse import OptionParser
Parser = OptionParser(usage="%prog [options] files\n\
   Type \"%prog -h\" for help", version="%prog 1.0")

Parser.add_option('-s', '--exitsuccess',  dest='exitsuccess',  action='store_true', default=False, help='Return success exit status in any case.')
Parser.add_option('-q', '--quiet',        dest='quiet',        action='store_true', default=False, help='Do not verbose commands.')
Parser.add_option('-d', '--debug',        dest='debug',        action='store_true', default=False, help='Debug mode, no commands execution.')

(Options, args) = Parser.parse_args()

if len(args) < 1: Parser.error('Not enough arguments provided.')

exit_status = 0

for file in args:
   status = 0
   # Use Unix or MS Windows slashes only:
   if sys.platform.find('win') == 0:
      file = file.replace('/','\\')
   else:
      file = file.replace('\\','/')

   if '*' in file:
      # Using native OS commands for files with mask "*"
      if not Options.quiet: print('Deleting file(s): "%s"' % file)
      if sys.platform.find('win') == 0:
         cmd = 'DEL /F /Q /S "%s"' % file
      else:
         file = file.replace('*','"*"')
         cmd = 'rm -rvf "%s"' % file
      if not Options.quiet:
         print('Executing system command:')
         print(cmd)
      if not Options.debug:
         status = os.system( cmd)
   elif os.path.isdir( file):
      # Removing folder(s):
      if not Options.quiet: print('Deleting directory: "%s"' % file)
      if not Options.debug:
         try:
            shutil.rmtree( file)
         except:
            print(str(sys.exc_info()[1]))
            status = 1
   elif os.path.isfile( file):
      # Removing file:
      if not Options.quiet: print('Deleting file: "%s"' % file)
      if not Options.debug:
         try:
            os.remove( file)
         except:
            print(str(sys.exc_info()[1]))
            status = 1
   else:
      # No such file or directory:
      print('Error: file(s) to delete not founded:')
      print(file)
      status = 1

   # Set exit status to error if it was any:
   if exit_status == 0 and status != 0: exit_status = status

if Options.exitsuccess: exit_status = 0

if not Options.quiet: print('Exit status = %d' % exit_status)

sys.exit(exit_status)
