#!/usr/bin/env python

import re
import subprocess

skip = [
r'.*[/\\]{1,1}override.*',
r'.*[/\\]{1,1}tmp',
r'.*[/\\]{1,1}bin',
r'.*[/\\]{1,1}pathmap_.*',
r'.*\.ncb',
r'.*\.suo',
r'.*\.vcproj\..*',
r'.*\.dll',
r'.*\\clear.cmd',
r'.*subversion.cmd',
r'.*doc.doxygen.output',
r'.*bin.exrjoin',
r'.*bin.ffmpeg.*',
r'.*bin.convert.*',
r'.*afanasy.trunk.*config.xml.*',
r'.*afanasy.trunk.*farm.xml.*',
r'.*afanasy.trunk.nonrootuser',
r'.*afanasy.trunk.python.check_pathmap.cmd',
r'.*/init/depends_.*',
r'.*plugins.maya.mll.*',
r'.*utilities.moviemaker.sequence.*',
r'.*utilities.moviemaker.logos.*',
r'.*utilities.qt.qt-.*',
r'.*utilities.python.[\dPs].*',
r'.*utilities.openexr.[slioD].*',
r'.*utilities.ffmpeg.x264',
r'.*utilities.ffmpeg.ffmpeg',
r'.*utilities.imagemagick.[Isli]',
r'.*utilities.site.temp',
r'.*utilities.release.output',
r'.*utilities.release.__all__',
r'.*utilities.release.__releases__',
r'.*utilities.release.cgru.*z',
r'.*[/\\]{1,1}icons.icons',
r'.*[/\\]{1,1}Makefile',
r'.*[/\\]{1,1}moc_.*',
r'.*project.qmake.project.pro.user',
r'.*project.cmake.*CMakeFiles',
r'.*project.cmake.*CMakeCache.*',
r'.*project.cmake.*\.cmake',
r'.*project.cmake.*\.vcproj',
r'.*project.cmake.*\.sln',
r'.*project.cmake.*[/\\]__',
r'.*project.cmake.*\\Release',
r'.*project.cmake.*\\.*\.dir',
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
   skipping = False
   for expr in exprs:
      if expr.match(line) != None:
         skipping = True
         skipped += 1
         break
   if skipping == False: print line,
   
output.close()

print '\nSkipped %d files.' % skipped
