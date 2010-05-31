#!/usr/bin/env python

import re
import subprocess

skip = [
r'.*[/\\]{1,1}override.*',
r'.*[/\\]{1,1}tmp.*',
r'.*afanasy.trunk.bin.*',
r'.*afanasy.trunk.config.xml.*',
r'.*afanasy.trunk.farm.xml.*',
r'.*plugins.maya.mll.*',
r'.*utilities.regexp.bin.*',
r'.*[/\\]{1,1}icons.icons',
r'.*[/\\]{1,1}Makefile',
r'.*[/\\]{1,1}moc_.*',
r'.*project.cmake.*CMakeFiles',
r'.*project.cmake.*CMakeCache.*',
r'.*project.cmake.*\.cmake',
r'.*project.cmake.*\.vcproj',
r'.*project.cmake.*\.sln',
r'.*project.cmake.*\.suo',
r'.*project.cmake.*[/\\]__',
r'.*project.cmake.*\\Release',
r'.*project.cmake.*\\.*\.dir',
r'.*project.cmake.*\\.*\.ncb',
r'.*afanasy/webvisor',
r'.*afanasy/plugins',
r'.*afanasy/doc',
r'.*afanasy/icons',
]

exprs = []
skip_len = len(skip)
print 'Skipping:'
for line in skip:
   exprs.append( re.compile(line))
   print line

output = subprocess.Popen(["svn", "st"], stdout=subprocess.PIPE).stdout

print '\nModifications:\n'
skipped = 0
for line in output:
#   print line,
   skipping = False
   for expr in exprs:
      if expr.match(line) != None:
         skipping = True
         skipped += 1
#         print 'match!'
         break
   if skipping == False: print line,
   
output.close()

print '\nSkipped %d files.' % skipped
