import os
import af

job = af.Job('After FX Test')

block = af.Block('Frames','afterfx')
block.setCommand('aerender -project "' + os.getcwd() + '\\AE Project.aep" -comp "Comp 1" -s @#@ -e @#@ -i 1 -mp -output "' + os.getcwd() + '/render/Comp 1_[####].jpg"')
block.setNumeric( 1, 10, 1)

job.blocks.append( block)

job.send()
