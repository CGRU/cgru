#!/usr/bin/env python

import os, sys

invalid_characters = '<>:;![]()$%^&*`\\|/?"\''
replace_character  = '_'

from optparse import OptionParser
parser = OptionParser(usage="%prog [options] root\ntype \"%prog -h\" for help", version="%prog 1.  0")

parser.add_option('-c', '--capital', dest='capital',  action='store_true', default=False, help='Check capital letters.')
parser.add_option('-s', '--spaces',  dest='spaces',   action='store_true', default=False, help='Check spaces.')
parser.add_option('-i', '--invalid', dest='invalid',  action='store_true', default=False, help='Check invalid characters.')
parser.add_option('-f', '--fix',     dest='fix',      action='store_true', default=False, help='Perform rename.')

(options, args) = parser.parse_args()

if not options.capital and not options.spaces and not options.invalid: parser.error('What to check, if no options provided?')

if len( args) < 1: parser.error('Not enough arguments provided.')
Root = args[0]

def fixName( name):
   fixedname = name
   for char in invalid_characters:
      fixedname = fixedname.replace( char, replace_character)
   return fixedname

def rename( oldname, newname):
   if options.fix: print 'Fixing:'
   print oldname
   print newname

   if newname == '':
      print 'Error: New path is empty.'
      return False
   if os.path.isdir( newname):
      print 'Error: New folder already exists.'
      return False
   if os.path.isfile( newname):
      print 'Error: New file already exists.'
      return False
   if os.path.lexists( newname):
      print 'Error: New path already exists.'
      return False

   if not options.fix:
      print
      return True

   result = True
   try:
      os.rename( oldname, newname)
   except:
      print str(sys.exc_info()[1])
      result = False

   if os.path.lexists( oldname):
      print 'Error: Old path still exists.'
      result = False

   if result: print 'Ok.'

   return result

for dirpath, dirnames, filenames in os.walk( Root, False, None):
   for dirname in dirnames:
      fixedname = dirname
      invalid = False

      # Check for spaces:
      if options.spaces:
         if fixedname.find(' ') != -1:
            fixedname = fixedname.replace(' ', replace_character)
            invalid = True

      # Check for invalid characterss:
      if options.invalid:
         for char in dirname:
            if char in invalid_characters:
               fixedname = fixName( fixedname)
               invalid = True
               break

      # Check for capital letters:
      if options.capital:
         loweredname = fixedname.lower()
         if fixedname != loweredname:
            fixedname = loweredname
            invalid = True

      # Skip if no rename needed:
      if not invalid: continue

      result = rename( os.path.join( dirpath, dirname), os.path.join( dirpath, fixedname))
      if result != True: sys.exit(1)
