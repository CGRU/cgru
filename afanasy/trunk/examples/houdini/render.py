#!/usr/bin/env python
# coding=utf8

import af
import os

rop = '/out/ifd'

job = af.Job('Houdini Test: IFD - Mantra')

block1 = af.Block( 'render ifd', 'mantra')
block1.setCommand('mantra -f render/scene.%04d.ifd')
block1.setTasksDependMask( 'generate ifd')
block1.setNumeric( 1, 10)

block2 = af.Block( 'generate ifd', 'hbatch')
block2.setCommand('hrender_af -s %1 -e %2 -b 1 scene.hip ' + rop)
block2.setNumeric( 1, 10, 5)

job.blocks.append( block1)
job.blocks.append( block2)

print
job.output( 1)
print

job.send()
