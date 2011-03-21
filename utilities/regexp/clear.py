#!/usr/bin/env python

import os
import shutil

removedirs = ['bin','CMakeFiles']
cleanfolders = ['.','src']
temps = ['moc_']
store = ['.cpp','.h','.pro','.svn','CMakeLists.txt','icons','doc','src','sh','py','cmd']

DEBUG = True
DEBUG = False

# Deletion procedure:
def delete( item):
   if os.path.isdir( item):
      print 'Deleting directory "%s"' % item
      if not DEBUG: shutil.rmtree( item)
   elif os.path.isfile( item):
      print 'Deleting file "%s"' % item
      if not DEBUG: os.remove( item)

for rmdir in removedirs: delete( rmdir)

for folder in cleanfolders:
   for item in os.listdir( folder):
      # Search for temporary items to delete
      todelete = False
      for tmp in temps:
         if item.rfind( tmp) != -1:
            todelete = True
            break
      # Delete temporary items
      if todelete:
         delete( os.path.join( folder, item))
         continue
      # Search for source items to store
      todelete = True
      for ext in store:
         if item.rfind( ext) != -1:
            todelete = False
            break
      if todelete: delete( os.path.join( folder, item))
