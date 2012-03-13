import hou
import af
import os

pwd=hou.pwd()
asset = os.getenv('ASSET', os.getcwd())
path = pwd.parm('files').eval()
start_frame = int(pwd.parm('f1').eval())
end_frame = int(pwd.parm('f2').eval())
#step_frame = int(pwd.parm('f3').eval())
command = pwd.parm('command').eval()
job_name = pwd.parm('jobname').eval()
depend_mask = pwd.parm('depend_mask').eval()

start_paused = pwd.parm('start_paused').eval()
enable_extended_parameters = pwd.parm('enable_extended_parameters').eval()
priority = pwd.parm('priority').eval()
maximum_hosts = pwd.parm('maximum_hosts').eval()
hosts_mask = pwd.parm('hosts_mask').eval()
hosts_mask_exclude = pwd.parm('hosts_mask_exclude').eval()

block_type='generic'
for service in af.services.__all__:
   if command.find( service) > -1: block_type=service

job = af.Job()
job.setName( job_name)

block = job.addBlock( block_type, block_type)
block.setWorkingDirectory( asset )
block.setCommand( command)
block.setNumeric( start_frame, end_frame, 1)

job.setDependMask( depend_mask)

if enable_extended_parameters:
   job.setPriority( priority)
   job.setMaxHosts( maximum_hosts)
   job.setHostsMask( hosts_mask)
   job.setHostsMaskExclude( hosts_mask_exclude)

if start_paused:
   job.offLine()

# Send job to Afanasy server.
job.send()
