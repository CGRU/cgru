# -*- coding: utf-8 -*-

import os
import shutil
import stat
import sys
import time

def createFolder( path, writeToAll = True):
   status = True
   if not os.path.isdir( path):
      try:
         os.makedirs( path )
      except:
         print(str(sys.exc_info()[1]))
         status = False
   if status and writeToAll:
      try:
         os.chmod( path, stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)
      except:
         print(str(sys.exc_info()[1]))
   return status


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
