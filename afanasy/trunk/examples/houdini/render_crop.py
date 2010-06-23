#!/usr/bin/env python
# coding=utf8

import af
import os

scene = 'scene.hip'
rop = '/out/ifd'
ifd = 'render/scene.%04d.ifd'
img = 'render/img.%04d.jpg'
f_start  = 1
f_finish = 10
divx = 3
divy = 2
tiles = divx * divy

job = af.Job('Houdini Test: Tile Render')

b_genifd = af.Block('generate ifd', 'hbatch')
b_genifd.setCommand('hrender_af -s %1 -e %2 -b 1 '+scene+' '+rop)
b_genifd.setNumeric( f_start, f_finish)

b_render = af.Block('render tiles', 'mantra')
b_render.setCommand('mantracroprender %(divx)d %(divy)d %%1' % vars() )
b_render.setTasksDependMask('generate ifd')
b_render.setFramesPerTask( -tiles)
for f in range( f_start, f_finish + 1):
   cmd = ' -f ' + ifd % f
   for t in range( 0, tiles):
      task = af.Task('%d tile %d' % (f,t))
      task.setCommand( str(t) + cmd)
      b_render.tasks.append( task)

exrjoin = os.environ['CGRU_LOCATION']
exrjoin = os.path.join( exrjoin, 'utilities')
exrjoin = os.path.join( exrjoin, 'exrjoin')
exrjoin = os.path.join( exrjoin, 'exrjoin')
exrjoin += ' %(divx)d %(divy)d' % vars()
b_exrjoin = af.Block('join tiles')
b_exrjoin.setCommand( exrjoin + ' ' + img + ' d', False)
b_exrjoin.setNumeric( f_start, f_finish)
b_exrjoin.setTasksDependMask('render tiles')

job.blocks.append( b_exrjoin )
job.blocks.append( b_render  )
job.blocks.append( b_genifd  )

print
job.output( 1)
print

job.send()
