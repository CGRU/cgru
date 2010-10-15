#!/usr/bin/env python
# coding=utf8

import af
import os

rop = '/out/image'

job = af.Job('Houdini Heavy')

block = af.Block( rop, 'hbatch')
block.setCommand('hrender_af -s %1 -e %2 scene.hip ' + rop)
block.setNumeric( 1, 10)

job.blocks.append( block)

print
job.output( 1)
print

job.send()
