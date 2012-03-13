#!/usr/bin/python

import os

import getenv

def walk():

   tuples = []
   # directories
   tuples.append([])
   # not html files
   tuples.append([])
   # html files
   tuples.append([])

   dircount = 0

   for dirpath, dirnames, filenames in os.walk( getenv.SRCDIR, True, None, True):
      exclude_directoty = False
      for findsub in getenv.DIRS_EXLUDE_FINDSUB:
         if dirpath.find( findsub) != -1:
            exclude_directoty = True
            break
      if exclude_directoty: continue

      include_directory = False
      for filename in filenames:
         filenamesize = len(filename)
         for inlude_fileext in getenv.DIRS_INLUDE_FILEEXT:
            extsize = len(inlude_fileext)
            if filenamesize <= extsize:
               continue
            if filename[filenamesize-extsize:filenamesize] == inlude_fileext:
               include_directory = True
               break
         if include_directory: break
      if include_directory == False: continue

#      print 'Procesing directory: "%s"' % dirpath
      tuples[0].append( dirpath)
      tuples[1].append( [])
      tuples[2].append( [])

      for filename in filenames:
         include_file = False
         filenamesize = len(filename)
         for inlude_fileext in getenv.DIRS_INLUDE_FILEEXT:
            extsize = len(inlude_fileext)
            if filenamesize <= extsize:
               continue
            if filename[filenamesize-extsize:filenamesize] == inlude_fileext:
               include_file = True
               break
         if include_file == False: continue

         filepath = os.path.join( dirpath, filename)
         if filename[filenamesize-4:filenamesize] != 'html':
            tuples[1][dircount].append( filename)
         else:
            fsrc = open( filepath, 'r')
            lines = fsrc.readlines()
            fsrc.close()
            tuples[2][dircount].append( filename)

      dircount += 1

   return tuples
