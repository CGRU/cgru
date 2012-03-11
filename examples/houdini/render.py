#!/usr/bin/env python
# coding=utf8

import af
import os

scene = 'scene.hip'
rop = '/out/ifd'
ifd  = 'render/scene.@####@.ifd'
ifdd = 'render/scene.%04d.ifd'
img  = 'render/img.@####@.exr'
imgd = 'render/img.%04d.exr'
f_start  = 1
f_finish = 10
divx = 3
divy = 2
tiles = divx * divy

job = af.Job('Houdini Test: Tile Render')

b_genifd = af.Block('generate ifd', 'hbatch')
b_genifd.setCommand('hrender_af -s @#@ -e @#@ '+scene+' '+rop)
b_genifd.setNumeric( f_start, f_finish)

b_render = af.Block('render tiles', 'mantra')
b_render.setCommand('mantrarender tc %(divx)d %(divy)d @#@' % vars() )
b_render.setTasksDependMask('generate ifd')
b_render.setFramesPerTask( -tiles)
for f in range( f_start, f_finish + 1):
   cmd = ' -R -f ' + ifdd % f
   for t in range( 0, tiles):
      task = af.Task('%d tile %d' % (f,t))
      task.setCommand( str(t) + cmd)
      task.setFiles( (imgd % f) + ('.tile_%d.exr' % t))
      b_render.tasks.append( task)

cmd = 'exrjoin %(divx)d %(divy)d %(img)s d && deletefiles %(ifd)s' % vars()
b_exrjoin = af.Block('join tiles')
b_exrjoin.setCommand( cmd, False)
b_exrjoin.setFiles( img)
b_exrjoin.setNumeric( f_start, f_finish)
b_exrjoin.setTasksDependMask('render tiles')

job.blocks.append( b_exrjoin )
job.blocks.append( b_render  )
job.blocks.append( b_genifd  )

print
job.output( 1)
print

job.send()
