#!/usr/bin/env python
# coding=utf8

import os
import af

job = af.Job('Nuke TCL Test')

block = af.Block( 'Write1', 'nuke')
block.setCommand('./nuke -X Write1 -x scene.nk %1,%2')
block.setNumeric( 1, 10, 2)
block.setCommandView('render/img.%04d.jpg')

job.blocks.append( block)

print
job.output( 1)
print

job.send()
