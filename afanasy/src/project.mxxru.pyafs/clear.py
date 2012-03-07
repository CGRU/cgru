#!/usr/bin/env python

import os
import shutil

todelete = 'tmp'

for afile in os.listdir( os.getcwd()):
   if afile.find( todelete) == 0:
      if os.path.isdir( afile):
         print('Deleting folder "%s"' % afile)
         shutil.rmtree( afile)
      elif os.path.isfile( afile):
         print('Deleting file "%s"' % afile)
         os.remove( afile)
