#!/usr/bin/env python

import os, sys

invalid_characters = ' :;![]()$%^&*`\\|/?"\''
replace_character  = '_'

from optparse import OptionParser
parser = OptionParser(usage="%prog [options] root\ntype \"%prog -h\" for help", version="%prog 1.  0")

parser.add_option('-f', '--fix', dest='fix', action='store_true', default=False, help='Perform rename')

(options, args) = parser.parse_args()

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
   print 'Ok.'

   return result

for dirpath, dirnames, filenames in os.walk( Root, False, None):
   for dirname in dirnames:
      fixedname = dirname
      invalid = False

      # Check for invalid characterss:
      for char in dirname:
         if char in invalid_characters:
            fixedname = fixName( fixedname)
            invalid = True
            break

      # Check for capital letters:
      loweredname = fixedname.lower()
      if fixedname != loweredname:
         fixedname = loweredname
         invalid = True

      # Skip if no rename needed:
      if not invalid: continue

      result = rename( os.path.join( dirpath, dirname), os.path.join( dirpath, fixedname))
      if result != True: sys.exit(1)
