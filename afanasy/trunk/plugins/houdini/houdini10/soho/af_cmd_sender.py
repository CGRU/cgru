import hou
import os

import af
import afhoudini

pwd=hou.pwd()
asset = os.getenv('ASSET', os.getcwd())
path = pwd.parm('files').eval()
start_frame = int(pwd.parm('f1').eval())
end_frame = int(pwd.parm('f2').eval())
#step_frame = int(pwd.parm('f3').eval())
command = afhoudini.pathToC( pwd.parm('command').evalAsStringAtFrame(start_frame), pwd.parm('command').evalAsStringAtFrame(end_frame))
job_name = pwd.parm('jobname').eval()
depend_mask = pwd.parm('depend_mask').eval()
preview = afhoudini.pathToC( pwd.parm('preview').evalAsStringAtFrame(start_frame), pwd.parm('preview').evalAsStringAtFrame(end_frame))

start_paused = pwd.parm('start_paused').eval()
platform = pwd.parm('platform').eval()
enable_extended_parameters = pwd.parm('enable_extended_parameters').eval()
priority = pwd.parm('priority').eval()
maximum_hosts = pwd.parm('maximum_hosts').eval()
capacity = pwd.parm('capacity').eval()
depend_mask_global = pwd.parm('depend_mask_global').eval()
hosts_mask = pwd.parm('hosts_mask').eval()
hosts_mask_exclude = pwd.parm('hosts_mask_exclude').eval()
capacity_min = pwd.parm('capacity_coefficient1').eval()
capacity_max = pwd.parm('capacity_coefficient2').eval()

block_type='generic'
block_type = command.split()[0]

job = af.Job()
job.setName( job_name)
if platform != '':
   if platform == 'any': job.setNeedOS('')
   else: job.setNeedOS( platform)

block = af.Block( block_type, block_type)
block.setWorkingDirectory( asset )
block.setCommand( command)
if preview != '': block.setCommandView( preview)
block.setNumeric( start_frame, end_frame, 1)
job.blocks.append( block)

job.setDependMask( depend_mask)

if enable_extended_parameters:
   job.setPriority( priority)
   if maximum_hosts > -1: job.setMaxHosts( maximum_hosts)
   if hosts_mask != '': job.setHostsMask( hosts_mask)
   if hosts_mask_exclude != '': job.setHostsMaskExclude( hosts_mask_exclude)
   if depend_mask_global != '': job.setDependMaskGlobal( depend_mask_global)
   block.setCapacity( capacity)
   block.setVariableCapacity( capacity_min, capacity_max)

if start_paused:
   job.offLine()

# Send job to Afanasy server.
job.send()

