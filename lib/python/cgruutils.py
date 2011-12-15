# -*- coding: utf-8 -*-

import os
import shutil
import stat
import sys
import time

import cgruconfig

def getIconFileName( iconname):
   icon_path = os.path.join( os.path.join( cgruconfig.VARS['CGRU_LOCATION'],'icons'))
   icon_paths = cgruconfig.VARS['icons_path']
   if icon_paths is None: icon_paths = icon_path
   if icon_paths.find(';') != -1: icon_paths = icon_paths.split(';')
   elif sys.platform.find('win') == -1: icon_paths = icon_paths.split(':')
   else: icon_paths = [icon_paths]
   if not icon_path in icon_paths: icon_paths.append( icon_path)
   for icon_path in icon_paths:
      icon_path = os.path.join( icon_path, iconname)
      if os.path.isfile( icon_path): return icon_path
      icon_path += '.png'
      if os.path.isfile( icon_path): return icon_path
   return None

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
