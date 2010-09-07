import os
import af

job = af.Job('3d MAX Test')

block = af.Block('Frames','max')
block.setCommand('3dsmaxcmd "' + os.getcwd() + '\\scene.max" -start:%1 -end:%2 -v:5 -showRFW:0')
block.setNumeric( 1, 10, 1)

job.blocks.append( block)

job.send()
