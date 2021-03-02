# -*- coding: utf-8 -*-

import af

scene    = 'scene.hip'
rop      = '/out/mantra_exr'
img_a    = 'render/image.@####@.exr'
img_d    = 'render/image.%04d.exr'
img_t    = 'render/image_tile%02d_.@####@.exr'
ifd_a    = img_a + '.ifd'
ifd_d    = img_d + '.ifd'
f_start  = 1
f_finish = 10
divx     = 3
divy     = 2
tiles    = divx * divy

job = af.Job('Houdini Test: Tile Render')

b_genifd = af.Block('generate ifd', 'hbatch')
b_genifd.setCommand('hrender_af -s @#@ -e @#@ %s %s' % (scene, rop))
b_genifd.setNumeric(f_start, f_finish)

b_render = af.Block('render tiles', 'mantra')
b_render.setCommand('mantra -t count=%dx%d,index=@#@' % (divx, divy))
b_render.setTasksDependMask('generate ifd')
b_render.setFramesPerTask(-tiles)
for f in range(f_start, f_finish + 1):
	for t in range(0, tiles):
		task = af.Task('%d tile %d' % (f, t))
		task.setCommand('%d -f %s' % (t, (ifd_d % f)))
		b_render.tasks.append(task)

cmd_stitch = 'itilestitch %s' % img_a
for t in range(0, tiles):
    cmd_stitch += ' ' + (img_t % t)
b_exrjoin = af.Block('join tiles','tiles_stitch')
b_exrjoin.setCommand(cmd_stitch, False)
b_exrjoin.setFiles(img_a)
b_exrjoin.setNumeric(f_start, f_finish)
b_exrjoin.setTasksDependMask('render tiles')

job.blocks.append(b_exrjoin)
job.blocks.append(b_render)
job.blocks.append(b_genifd)

print('')
job.output()
print('')

job.send()
