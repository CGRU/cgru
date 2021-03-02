# -*- coding: utf-8 -*-

import af

scene    = 'scene.hip'
rop      = '/out/mantra_jpg'
f_start  = 1
f_finish = 10

job = af.Job('Houdini Test: Simple Render')

block = af.Block('render', 'hbatch_mantra')
block.setCommand('hrender_af -s @#@ -e @#@ %s %s' % (scene, rop))
block.setNumeric(f_start, f_finish)

job.blocks.append(block)

print('')
job.output()
print('')

job.send()
