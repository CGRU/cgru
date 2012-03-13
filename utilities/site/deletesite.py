#!/usr/bin/python

import os
import shutil

import getenv
import walk

srcdir = getenv.srcdir()

paths = walk.walk( srcdir)
dircount = 0

for dirpath in paths[0]:

   print 'Procesing directory: "%s"' % dirpath
   for filename in paths[2][dircount]:
      filepath = os.path.join( dirpath, filename)
      print 'Removing "%s"' % filepath
      os.remove( filepath)
      os.system( 'svn revert ' + filepath)

   dircount += 1
