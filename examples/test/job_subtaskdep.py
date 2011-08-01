#!/usr/bin/env python

import af

job = af.Job('Job with sub task dependence')

block1 = af.Block('multiframe_block')
block1.setCommand('./task.py -s @#@ -e @#@ -t 10')
block1.setNumeric( 10, 20, 11)
block1.setDependSubTask()

block2 = af.Block('perframe_block')
block2.setCommand('./task.py -s @#@ -e @#@')
block2.setNumeric( 10, 20, 1)
block2.setTasksDependMask('multiframe_block')

job.blocks.append(block2)
job.blocks.append(block1)

job.send()
