#!/usr/bin/env python

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

   afanasy_branches_path = os.path.join( getenv.SRCDIR, 'afanasy')

   for dirpath, dirnames, filenames in os.walk( getenv.SRCDIR, True, None, False):

      # Check afanasy branch:
      if dirpath.find( afanasy_branches_path) == 0:
         folder = dirpath[len(afanasy_branches_path):]
         if len(folder) < 1: continue
         while folder[0] == '/' and len(folder): folder = folder[1:]
         while folder != os.path.dirname( folder) and len(os.path.dirname( folder)): folder = os.path.dirname( folder)
         if folder.find( getenv.AFANASY) == -1: continue

      # Check exclude strings:
      exclude_directoty = False
      for findsub in getenv.DIRS_EXLUDE_FINDSUB:
         if dirpath.find( findsub) != -1:
            exclude_directoty = True
            break
      if exclude_directoty: continue

      # Check include strings:
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
