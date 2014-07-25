# -*- coding: utf-8 -*-
import os
import af

job = af.Job('3d MAX Test')

block = af.Block('Frames', 'max')
block.setCommand(
	'3dsmaxcmd "%s\\scene.max" -start:@#@ -end:@#@ -v:5 -showRFW:0 '
	'-o:"render/from_script.0000.jpg"' % os.getcwd()
)
block.setNumeric(1, 10, 1)

job.blocks.append(block)

job.send()
