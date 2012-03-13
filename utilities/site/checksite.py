#!/usr/bin/python

import os

import getenv
import walk

def errorFilename( filename):
   print 'ERROR: filename "%s":' % filename
def warningFilename( filename):
   print 'WARNING: filename "%s":' % filename

def find( lines, line, exact):
   for l in lines:
      if exact:
         if l == line:
            return True
      else:
         if l.find( line) > -1:
            return True
   return False

def process_title( filename, lines):
   stylesheet = False
   title = False
   for line in lines:
      if line.find('</title>') > -1:
         title = True
         if stylesheet == False:
            warningFilename( filename)
            print 'No stylesheet before title.'
      if line.find('stylesheet') > -1:
         stylesheet = True
   if title == False:
      errorFilename( filename)
      print 'No title.'

def process_header( filename, lines):
   if find( lines, '<!--HEADER-->\n', True) == False:
      if find( lines, '<body>\n', True) == False:
         errorFilename( filename)
         print '<body> is invalid.'

def process_footer( filename, lines):
   if find( lines, '<!--FOOTER-->\n', True) == False:
      if find( lines, '</body>\n', True) == False:
         errorFilename( filename)
         print '</body> is invalid.'

srcdir = getenv.srcdir()
paths = walk.walk( srcdir)
dircount = 0
for dirpath in paths[0]:

   print 'Procesing directory: "%s"' % dirpath
   for filename in paths[2][dircount]:
      filepath = os.path.join( dirpath, filename)
      fsrc = open( filepath, 'r')
      lines_src = fsrc.readlines()
      fsrc.close()

      if find( lines_src, '<!--SKIP-->', False):
         print 'Skipping: "%s"' % filepath
         continue

      print 'Procesing: "%s"' % filepath
      process_title(  filepath, lines_src)
      process_header( filepath, lines_src)
      process_footer( filepath, lines_src)

   dircount += 1

process_title
