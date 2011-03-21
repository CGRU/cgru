#!/usr/bin/env python

import re
import subprocess

skip = [
r'.*[/\\]{1,1}override.*',
r'.*[/\\]{1,1}tmp',
r'.*[/\\]{1,1}bin',
r'.*[/\\]{1,1}pathmap_.*',
r'.*\.zip',
r'.*\.7z',
r'.*\.gz',
r'.*\.gem',
r'.*\.ncb',
r'.*\.suo',
r'.*\.dll',
r'.*\.exe',
r'.*\.msi',
r'.*\\clear.cmd',
r'.*subversion.cmd',
r'.*utilities.moviemaker.sequence.*',
r'.*utilities.moviemaker.logos.*',
r'.*utilities.python.[\dPs].*',
r'.*[/\\]{1,1}moc_.*',
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
   skipping = False
   for expr in exprs:
      if expr.match(line) != None:
         skipping = True
         skipped += 1
         break
   if skipping == False: print line,
   
output.close()

print '\nSkipped %d files.' % skipped
