#!/usr/bin/env python

import os
import shutil

todelete = 'tmp'

for folder in os.listdir( os.getcwd()):
   if os.path.isdir( folder):
      if folder.find( todelete) == 0:
         print 'Deleting "%s"' % folder
         shutil.rmtree( folder)
