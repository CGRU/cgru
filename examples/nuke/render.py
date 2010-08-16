#!/usr/bin/env python
# coding=utf8

import os
import af

job = af.Job('Nuke Test')

block1 = af.Block( 'preview', 'nuke')
block1.setCommand('nuke -X preview -x scene.nk %1,%2')
block1.setNumeric( 1, 20, 2)
block1.setTasksDependMask( 'final')
block1.setFiles('render/preview.%04d.jpg')

block2 = af.Block( 'final', 'nuke')
block2.setCommand('nuke -X final -x scene.nk %1,%2')
block2.setNumeric( 1, 20, 1)
block2.setTasksDependMask( 'key|back')
block2.setFiles('render/final.%04d.exr')

block3 = af.Block( 'key', 'nuke')
block3.setCommand('nuke -X key -x scene.nk %1,%2')
block3.setNumeric( 1, 20, 3)
block3.setFiles('render/key.%04d.exr')

block4 = af.Block( 'back', 'nuke')
block4.setCommand('nuke -X back -x scene.nk %1,%2')
block4.setNumeric( 1, 20, 3)
block4.setFiles('render/back.%04d.exr')

job.blocks.append( block1)
job.blocks.append( block2)
job.blocks.append( block3)
job.blocks.append( block4)

print
job.output( 1)
print

job.send()
