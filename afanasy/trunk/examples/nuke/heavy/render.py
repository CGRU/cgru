#!/usr/bin/env python
# coding=utf8

import os
import af

job = af.Job('Nuke Heavy')

block = af.Block( 'write', 'nuke')
block.setCommand('nuke -X Write1 -x scene.nk %1,%2')
block.setNumeric( 1, 10, 1)

job.blocks.append( block)

print
job.output( 1)
print

job.send()
