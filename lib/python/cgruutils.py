# -*- coding: utf-8 -*-

import os
import time
import shutil

def copyJobFile( fileName, jobName = '', fileExtension = ''):
   copyFile = ''
   if not os.path.isfile( fileName):
      print('Error: no file "%s"' % fileName)
   else:
      copyFile = fileName
      if jobName != '': copyFile += '.' + jobName
      ftime = time.time()
      copyFile += '.' + time.strftime('%m%d-%H%M%S-') + str(ftime - int(ftime))[2:5]
      if fileExtension != '': copyFile += '.' + fileExtension
      print('Copying to ' + copyFile)
      try:
         shutil.copyfile( fileName, copyFile)
      except:
         print(str(sys.exc_info()[1]))
         copyFile = ''
   return copyFile
