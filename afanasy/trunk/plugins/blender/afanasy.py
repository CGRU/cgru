#!BPY
"""
Name: 'Afanasy...'
Blender: 243
Group: 'Render'
"""
import Blender
import os
import time
import af

def main():

#  Get some scene parameters:

   rctx = Blender.Scene.GetCurrent().getRenderingContext()

   anim_start = rctx.sFrame
   anim_end   = rctx.eFrame
   engineid   = rctx.renderer
   enginename = ''
   if   engineid == Blender.Scene.Render.INTERNAL: enginename = 'internal'
   elif engineid == Blender.Scene.Render.YAFRAY:   enginename = 'yafray'

   filename = Blender.Get('filename')
   scene = Blender.sys.basename( filename)
   point = scene.rfind('.')
   if point > 1: scene = scene[0:point]

#  GUI:

   Draw = Blender.Draw
   job_name = Draw.Create( scene)
   start_frame = Draw.Create( anim_start)
   end_frame = Draw.Create( anim_end)
   per_host = Draw.Create( 1)
   max_hosts = Draw.Create( -1)
   priority = Draw.Create( 99)
   depend_mask = Draw.Create( '')
   hosts_mask = Draw.Create( '')
   hosts_mask_exclude = Draw.Create( '')
   need_properties = Draw.Create( '')
   paused = Draw.Create( 0)

   if not Draw.PupBlock('Send Job to Afanasy:', [\
   ('Job Name:', job_name,  0, 99, 'Job name'),\
   ('Start Frame:', start_frame,  anim_start, anim_end, 'Start frame'),\
   ('End Frame:', end_frame,  anim_start, anim_end, 'End frame'),\
   ('Per Host:', per_host,  1, anim_end-anim_start, 'Number of frames per render host (per task)'),\
   ('Max Hosts:', max_hosts, -1, 999, 'Maximum hosts job can take'),\
   ('Priority:', priority, 1, 200, 'Job priority (running order)'),\
   ('Depend Mask:', depend_mask, 0, 99, 'Jobs names pattern to wait'),\
   ('Hosts Mask:', hosts_mask, 0, 99, 'Hosts name pattern job can take'),\
   ('Hosts Exclude:', hosts_mask_exclude, 0, 99, 'Hosts name pattern job can not take'),\
   ('Properties:', need_properties, 0, 99, 'Job hosts required properties'),\
   ('Start Paused', paused,  'Start job paused'),\
   ]):
      return

#  Save temporary file to render:

   ftime = time.time()
   tmpname = filename + '.' + job_name.val + time.strftime('.%m%d-%H%M%S-') + str(ftime - int(ftime))[2:5] + '.blend'
   Blender.Save(tmpname)

#  Create a job:

   job = af.Job( job_name.val)

   # Set block service type:
   blockname = 'blender'
   servicename = 'blender'
   if enginename != '': blockname = 'engine:' + enginename
   block = af.Block( blockname, servicename)
   job.blocks.append( block)
   # Set block command and frame range:
   block.setCommand('blender -b %(tmpname)s -s @#@ -e @#@ -a'  % vars())
   block.setNumeric( start_frame.val, end_frame.val, per_host.val)
   # Set job running parameters:
   job.setMaxRunningTasks( max_hosts.val )
   job.setPriority( priority.val )
   job.setDependMask( depend_mask.val )
   job.setHostsMask( hosts_mask.val )
   job.setHostsMaskExclude( hosts_mask_exclude.val )
   job.setNeedProperties( need_properties.val )
   if paused.val: job.offLine()
   # Make server to delete temporary file after job deletion:
   job.setCmdPost('deletefiles "%s"' % os.path.abspath(tmpname))

#  Sending job to server:

   job.send()

main()
