# -*- coding: utf-8 -*-
import os
import af

job = af.Job('After FX Test')

block = af.Block('Frames', 'afterfx')
block.setCommand(
	'aerender -project "%(cwd)s\\comp.aep" -comp "Comp 1" -s @#@ '
	'-e @#@ -i 1 -mp -output "%(cwd)s/render/Comp 1_[####].jpg"' % {
		'cwd': os.getcwd()
	}
)
block.setNumeric(1, 10, 1)

job.blocks.append(block)

job.send()
