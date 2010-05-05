#!/usr/bin/env python

import os
import shutil

bins = 'bin'
sources = 'src'
temps = ['moc_']
store = ['.cpp','.h','.pro','.svn']

DEBUG = True
DEBUG = False

if os.path.isdir( bins): shutil.rmtree( bins)

# Deletion procedure:
def delete( item):
   if os.path.isdir( item):
      print 'Deleting directory "%s"' % item
      if not DEBUG: shutil.rmtree( item)
   else:
      print 'Deleting file "%s"' % item
      if not DEBUG: os.remove( item)

for item in os.listdir( sources):
   if DEBUG: print 'item = ' + item
   # Search for temporary items to delete
   todelete = False
   for tmp in temps:
      if item.rfind( tmp) != -1:
         todelete = True
         break
   # Delete temporary items
   if todelete:
      delete( os.path.join( sources, item))
      continue
   # Search for source items to store
   todelete = True
   for ext in store:
      if item.rfind( ext) != -1:
         todelete = False
         break
   if todelete: delete( os.path.join( sources, item))
