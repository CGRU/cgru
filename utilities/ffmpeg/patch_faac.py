#!/usr/bin/env python

filename = 'common/mp4v2/mpeg4ip.h'

import os, shutil, sys

file = open( filename,'r')
lines = file.readlines()
file.close()

shutil.move( filename, filename + '.old')

file = open( filename,'w')
line_num = 1
for line in lines:
   if line_num == 126:
      line = '// ' + line
      print('%d: %s' % (line_num, line))
   file.write( line)
   line_num += 1
file.close()
