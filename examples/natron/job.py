#!/usr/bin/env python

import af
import os

scene = os.path.join( os.getcwd(),'scene.ntp')

job = af.Job('Natron job.py')

block = af.Block('w_jpg','natron')
block.setCommand('natron -b -w w_jpg @#@-@#@ ' + scene)
block.setNumeric(1, 20, 2)

job.blocks.append( block)

job.send()

