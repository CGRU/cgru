# -*- coding: utf-8 -*-

import os
import re
import time
import shutil
import string

def cPathFrom2( path_a, path_b):
   path = path_a

   len_a = len(path_a)
   len_b = len(path_b)
   len_min = len_a
   if len_min > len_b: len_min = len_b
   if len_min < 1: return path

   len_begin = -1
   for c in range( len_min):
      if path_a[c] == path_b[c]: continue
      len_begin = c
      break
   if len_begin < 1: return path

   len_end = -1
   for c in range( len_min):
      if path_a[len_a-c-1] == path_b[len_b-c-1]: continue
      len_end = c
      break
   if len_end < 1: return path

   for c in range( len_begin):
      if path_a[len_begin-c] in string.digits: continue
      len_begin = len_begin - c + 1
      break

   for c in range( len_end):
      if path_a[len_a-len_end+c] in string.digits: continue
      len_end = len_end - c
      break

   padding = ''
   if len_a == len_b:
      digits = len_a - len_begin - len_end
      if digits > 1: padding = '0' + str(digits)
   
   path = path_a[0:len_begin] + '%' + padding + 'd' + path_a[len_a-len_end:len_a]

   return path

def cPathFrom1( path):
   newpath = path
   dot = path.rfind('.')
   if dot == -1: dot = len(path)
   digits = re.findall( r'\d{1,}', path[:dot])
   if len( digits):
      digits = digits[-1]
      pos = path.rfind(digits)
      if pos != -1:
         newpath = path[:pos]
         newpath += '%'
         padding = len(digits)
         if padding > 1:
            newpath += '0' + str(padding)
         newpath += 'd'
         newpath += path[pos+padding:]
   return newpath

def copyJobFile( fileName, jobName = '', fileExtension = ''):
   copyFile = ''
   if not os.path.isfile( fileName):
      print 'Error: no file "%s"' % fileName
   else:
      copyFile = fileName
      if jobName != '': copyFile += '.' + jobName
      ftime = time.time()
      copyFile += '.' + time.strftime('%m%d-%H%M%S-') + str(ftime - int(ftime))[2:5]
      if fileExtension != '': copyFile += '.' + fileExtension
      print 'Copying to ' + copyFile
      try:
         shutil.copyfile( fileName, copyFile)
      except:
         print str(sys.exc_info()[1])
         copyFile = ''
   return copyFile
