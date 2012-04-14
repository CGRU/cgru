#!/usr/bin/env python

import os
import shutil
import sys

store = ['.svn','.gitignore','build.sh','override.sh','clear.py','project.pro','win_build_mingw.cmd','win_build_msvc.cmd']
projects = ['libafanasy','libafqt','libafsql','cmd','server','render','talk','watch','monitor']

folders = []
if len(sys.argv) > 1:
   for i in range( 1, len(sys.argv)):
      folders.append( sys.argv[i])
else:
   folders = projects
for project in projects:
   if not project in folders:
      store.append( project)

DEBUG = True
DEBUG = False

def delete( item):
   if os.path.isdir( item):
      print 'Deleting directory "%s"' % item
      if not DEBUG: shutil.rmtree( item)
   else:
      print 'Deleting file "%s"' % item
      if not DEBUG: os.remove( item)

for item in os.listdir('.'):
   if item in store: continue
   if item in folders:
      project = item
      for item in os.listdir( project):
         if item == ('%s.pro' % project) or item == '.svn': continue
         delete( os.path.join( project, item))
      continue
   delete( item)
