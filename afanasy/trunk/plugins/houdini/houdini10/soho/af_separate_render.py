import hou
import os
import time

import af
import afhoudini

afnode = hou.pwd()
f_start = int( afnode.parm('f1').eval())
f_finish = int( afnode.parm('f2').eval())
take = afnode.parm('take').eval()
#step_frame = int(pwd.parm('f3').eval())
job_name = afnode.parm('jobname').eval()
rop = afnode.parm('rop').eval()
run_rop = afnode.parm('run_rop').eval()
read_rop = afnode.parm('read_rop').eval()
depend_mask = afnode.parm('depend_mask').eval()
tile_render = afnode.parm('tile_render').eval()
divx = afnode.parm('divx').eval()
divy = afnode.parm('divy').eval()
temp_images = afnode.parm('temp_images').eval()
delete_files = afnode.parm('delete_files').eval()

start_paused = afnode.parm('start_paused').eval()
platform = afnode.parm('platform').eval()
enable_extended_parameters = afnode.parm('enable_extended_parameters').eval()
priority = afnode.parm('priority').eval()
maximum_hosts = afnode.parm('maximum_hosts').eval()
capacity = afnode.parm('capacity').eval()
capacity_min = afnode.parm('capacity_coefficient1').eval()
capacity_max = afnode.parm('capacity_coefficient2').eval()
capacity_generate = afnode.parm('capacity_generate').eval()
capacity_join = afnode.parm('capacity_join').eval()
depend_mask_global = afnode.parm('depend_mask_global').eval()
hosts_mask = afnode.parm('hosts_mask').eval()
hosts_mask_exclude = afnode.parm('hosts_mask_exclude').eval()

if read_rop or run_rop:
   ropnode = hou.node( rop)
   if not ropnode:
      soho.error('Can`t find ROP for processing')
   if not isinstance( ropnode, hou.RopNode):
      soho.error('%s is not a ROP node' % rop)

job = af.Job()
job.setName( job_name)
if platform != '':
   if platform == 'any': job.setNeedOS('')
   else: job.setNeedOS( platform)

if run_rop:
   rop_setdiskfile = False
   if ropnode.parm('soho_outputmode').eval() == 0:
      # Set outpu mode to produce ifd files:
      ropnode.parm('soho_outputmode').set(1)
      rop_setdiskfile = True
      ropnode.parm('soho_diskfile').set( ropnode.parm('vm_picture').unexpandedString() + '.ifd')

   # Calculate temporary hip name:
   ftime = time.time()
   tmphip = hou.hipFile.name() + '_' + job_name + time.strftime('.%m%d-%H%M%S-') + str(ftime - int(ftime))[2:5] + ".hip"

   # Use mwrite, because hou.hipFile.save(tmphip)
   # changes current scene file name to tmphip
   hou.hscript('mwrite -n %s' % tmphip)

   # Set output mode back if was enabled:
   if rop_setdiskfile: ropnode.parm('soho_outputmode').set(0)

   job.setCmdPost('rm ' + tmphip)
   b_generate = af.Block('generate', 'hbatch')
   b_generate.setCommand('hrender_af -s %1 -e %2 -b 1 -t '+take+' '+tmphip+' '+rop)
   b_generate.setNumeric( f_start, f_finish)

if read_rop:
   images = ropnode.parm('vm_picture')
   files  = ropnode.parm('soho_diskfile')
   afnode.parm('images').set(images.unexpandedString())
   afnode.parm('files' ).set( files.unexpandedString())

images = afhoudini.pathToC( afnode.parm('images').evalAsStringAtFrame(f_start), afnode.parm('images').evalAsStringAtFrame(f_finish))
files  = afhoudini.pathToC( afnode.parm('files' ).evalAsStringAtFrame(f_start), afnode.parm('files' ).evalAsStringAtFrame(f_finish))

command = 'mantra'
tiles = divx * divy
b_render = af.Block('render', command)
if run_rop: b_render.setTasksDependMask('generate')
if tile_render or delete_files or temp_images: command = 'mantrarender '
if delete_files and not tile_render: command += 'd'
if temp_images: command += 't'
if tile_render:
   command += 'c %(divx)d %(divy)d' % vars()
   b_render.setCommand( command + ' %1')
   b_render.setFramesPerTask( -tiles)
   for frame in range( f_start, f_finish + 1):
      arguments = afnode.parm('arguments').evalAsStringAtFrame( frame)
      for tile in range( 0, tiles):
         task = af.Task('%d tile %d' % ( frame, tile))
         task.setCommand( '%d -R %s' % ( tile, arguments))
         b_render.tasks.append( task)
else:
   if delete_files or temp_images: command += ' -R '
   else: command +=  ' -v A '
   arguments = afhoudini.pathToC( afnode.parm('arguments').evalAsStringAtFrame(f_start), afnode.parm('arguments').evalAsStringAtFrame(f_finish))
   b_render.setCommand( command + arguments)
   b_render.setCommandView( images)
   b_render.setNumeric( f_start, f_finish)

if tile_render:
   cmd = 'exrjoin %(divx)d %(divy)d %(images)s d' % vars()
   if delete_files: cmd += '; rm %s' % files
   b_join = af.Block('join', 'generic')
   b_join.setTasksDependMask('render')
   b_join.setCommand( cmd)
   b_join.setCommandView( images)
   b_join.setNumeric( f_start, f_finish)

if read_rop:
   afnode.parm('images').set('')
   afnode.parm('files' ).set('')

if tile_render: job.blocks.append( b_join)
job.blocks.append( b_render)
if run_rop: job.blocks.append( b_generate)

job.setDependMask( depend_mask)

if enable_extended_parameters:
   job.setPriority( priority)
   if maximum_hosts > -1: job.setMaxHosts( maximum_hosts)
   if hosts_mask != '': job.setHostsMask( hosts_mask)
   if hosts_mask_exclude != '': job.setHostsMaskExclude( hosts_mask_exclude)
   if depend_mask_global != '': job.setDependMaskGlobal( depend_mask_global)
   if run_rop: b_generate.setCapacity( capacity_generate)
   if tile_render: b_join.setCapacity( capacity_join)
   b_render.setCapacity( capacity)
   b_render.setVariableCapacity( capacity_min, capacity_max)

if start_paused:
   job.offLine()

# Send job to Afanasy server.
job.send()
