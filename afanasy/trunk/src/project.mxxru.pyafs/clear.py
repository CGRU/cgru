#!/usr/bin/env python

import os
import shutil

todelete = 'tmp'

for afile in os.listdir( os.getcwd()):
   if afile.find( todelete) == 0:
      print 'Deleting "%s"' % afile
      shutil.rmtree( afile)
