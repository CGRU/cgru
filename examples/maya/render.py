#!/usr/bin/env python
# coding=utf8

import os
import af

job = af.Job('Maya Example')

block = af.Block('render', 'maya')
block.setCommand('maya -batch -file ' + os.getcwd() + '/scene.mb -command "afanasyBatch(@#@,@#@,1,1)"')
block.setNumeric( 1, 5, 2)

job.blocks.append(block)

print
job.output( True)
print

job.send()
