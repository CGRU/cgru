#!/usr/bin/env python

import os
import shutil

store = ['FindPostgreSQL.cmake','.gitignore','.svn','build.sh','build_ostype.sh','override.sh','override.cmd','clear.py','clear.cmd','CMakeLists.txt','win_build_msvc_pyaf.cmd','win_build_msvc.cmd']
projects = ['libafanasy','libafqt','libafsql','cmd','server','render','talk','watch','monitor']

DEBUG = True
DEBUG = False

def delete( item):
   if os.path.isdir( item):
      print('Deleting directory "%s"' % item)
      if not DEBUG: shutil.rmtree( item)
   else:
      print('Deleting file "%s"' % item)
      if not DEBUG: os.remove( item)

for item in os.listdir('.'):
   if item in store: continue
   if item in projects:
      project = item
      for item in os.listdir( project):
         if item in store: continue
         delete( os.path.join( project, item))
      continue
   delete( item)
