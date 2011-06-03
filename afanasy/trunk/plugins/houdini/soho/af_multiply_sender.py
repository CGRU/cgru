import soho
from soho import SohoParm
import hou
import os
import re
import time

import af
import afcommon

genParms = {
    'trange'                     : SohoParm('trange'                    ,'int'   , [0],          False),
    'f'                          : SohoParm('f'                         ,'int'   , [1,1,1],      False),
    'jobname'                    : SohoParm('jobname'                   ,'string', [''],         False),
    'take'                       : SohoParm('take'                      ,'string', ['_current_'],False),

    'gen_name'                   : SohoParm('object:name'               ,'string', [''], False, key='gen_name'),
    'now'                        : SohoParm('state:time'                ,'real'  , [0],  False, key='now'),
    'fps'                        : SohoParm('state:fps'                 ,'real'  , [24], False, key='fps'),
    'hip'                        : SohoParm('$HIP'                      ,'string', [''], False, key='hip'),
    'hipname'                    : SohoParm('$HIPNAME'                  ,'string', [''], False, key='hipname'),

    'enable_extended_parameters' : SohoParm('enable_extended_parameters','int'   , [0]    , False),
    'depend_mask'                : SohoParm('depend_mask'               ,'string', ['']   , False),
    'depend_mask_global'         : SohoParm('depend_mask_global'        ,'string', ['']   , False),
    'priority'                   : SohoParm('priority'                  ,'int'   , [99]   , False),
    'maximum_hosts'              : SohoParm('maximum_hosts'             ,'int'   , [-1]   , False),
    'capacity'                   : SohoParm('capacity'                  ,'int'   , [-1]   , False),
    'hosts_mask'                 : SohoParm('hosts_mask'                ,'string', ['']   , False),
    'hosts_mask_exclude'         : SohoParm('hosts_mask_exclude'        ,'string', ['']   , False),
    'start_paused'               : SohoParm('start_paused'              ,'int'   , [0]    , False),
    'platform'                   : SohoParm('platform'                  ,'string', ['']   , False),
}
plist = soho.evaluate(genParms)

job_trange  = plist['trange'].Value[0]
f           = plist['f'].Value
gen_name    = plist['gen_name'].Value[0]
job_name    = plist['jobname'].Value[0]
take        = plist['take'].Value[0]
FPS         = plist['fps'].Value[0]
now         = plist['now'].Value[0]
hip         = plist['hip'].Value[0]
hipname     = plist['hipname'].Value[0]

enable_extended_parameters = plist['enable_extended_parameters'].Value[0]
job_depend_mask            = plist['depend_mask'].Value[0]
job_depend_mask_global     = plist['depend_mask_global'].Value[0]
job_priority               = plist['priority'].Value[0]
job_maximum_hosts          = plist['maximum_hosts'].Value[0]
job_capacity               = plist['capacity'].Value[0]
job_hosts_mask             = plist['hosts_mask'].Value[0]
job_hosts_mask_exclude     = plist['hosts_mask_exclude'].Value[0]
job_start_paused           = plist['start_paused'].Value[0]
platform                   = plist['platform'].Value[0]

soho.initialize(now, '')
soho.lockObjects(now)


if len(f) == 1:
   f.append(f[0])
if len(f) == 2:
   f.append(1)
if job_trange == 0:
   f[0] = hou.frame()
   f[1] = f[0]
   f[2] = 1

job_start, job_end, job_by = f
if job_end < job_start: job_end = job_start
if job_by < 1: job_by = 1

ftime = time.time()
tmphip = hip + '/' + job_name + time.strftime('.%m%d-%H%M%S-') + str(ftime - int(ftime))[2:5] + ".hip"
asset = os.getenv('ASSET', os.getcwd())

job = af.Job( job_name)
job.setCmdPost('deletefiles "%s"' % tmphip)
if enable_extended_parameters:
   if job_maximum_hosts > -1: job.setMaxHosts( job_maximum_hosts)
   if job_depend_mask != '': job.setDependMask( job_depend_mask)
   if job_depend_mask_global != '': job.setDependMaskGlobal( job_depend_mask_global)
   if job_priority > -1: job.setPriority( job_priority)
   if job_hosts_mask != '': job.setHostsMask( job_hosts_mask)
   if job_hosts_mask_exclude != '': job.setHostsMaskExclude( job_hosts_mask_exclude)
if job_start_paused:
   job.offLine()
if platform != '':
   if platform == 'any': job.setNeedOS('')
   else: job.setNeedOS( platform)

inputs = []
inputs.extend( hou.pwd().inputs())
inputs.reverse()
newInputs=[]
for i in inputs:
   if not i.isBypassed():
      newInputs.append(i)
inputs=newInputs

lenInputs = len(inputs)

for i in xrange(0,lenInputs):
   node = inputs[i]
   name = node.name()
   nodeType = node.type().name()
   if nodeType == 'afanasy': soho.error('Do not connect "afanasy" node.')
   if nodeType == 'af_multiply_sender': soho.error('Do not connect other "af_multiply_sender" node.')
   block_driver = node.path()
   block_start = job_start
   block_end   = job_end
   block_by    = job_by
   block_trange = node.parm('trange').eval()
   if block_trange:
      block_start = int(node.parm('f1').eval())
      block_end   = int(node.parm('f2').eval())
      block_by    = int(node.parm('f3').eval())
#   print name+' %(block_start)d %(block_end)d %(block_by)d' % vars()
   block_cmd  = 'hrender_af -s @#@ -e @#@ --by 1 -t %(take)s %(tmphip)s %(block_driver)s' % vars()
   block_type = 'hbatch'
   preview = ''

   depend_mask = ''
   has_depend_mask = False
   depend_mask_parm = node.parm('depend_mask')
   if depend_mask_parm:
      depend_mask = depend_mask_parm.eval()
      if depend_mask != None:
         if depend_mask != '': has_depend_mask = True

   block_capacity = job_capacity
   block_capmin = -1
   block_capmax = -1
   capacity_parm = node.parm('capacity')
   if capacity_parm:
      block_capacity = capacity_parm.eval()
      if block_capacity < 0: block_capacity = job_capacity
   capacity_coefficient_min_parm = node.parm('capacity_coefficient1')
   if capacity_coefficient_min_parm:
      block_capmin = capacity_coefficient_min_parm.eval()
   capacity_coefficient_max_parm = node.parm('capacity_coefficient2')
   if capacity_coefficient_max_parm:
      block_capmax = capacity_coefficient_max_parm.eval()

   if nodeType == 'ifd':
      block_type += '_mantra'
      preview = afcommon.patternFromPaths( node.parm('vm_picture').evalAsStringAtFrame(block_start), node.parm('vm_picture').evalAsStringAtFrame(block_end))
   elif nodeType == 'rib':
      block_type += '_prman'
      preview = afcommon.patternFromPaths( node.parm('ri_display').evalAsStringAtFrame(block_start), node.parm('ri_display').evalAsStringAtFrame(block_end))
   elif nodeType == 'af_cmd_sender':
      preview  =  afcommon.patternFromPaths( node.parm('preview').evalAsStringAtFrame(block_start), node.parm('preview').evalAsStringAtFrame(block_end))
      block_cmd = afcommon.patternFromPaths( node.parm('command').evalAsStringAtFrame(block_start), node.parm('command').evalAsStringAtFrame(block_end))
      if block_cmd.find('mantra') > -1:
         block_type = 'mantra'
      elif  block_cmd.find('prman') > -1:
         block_type = 'prman'
      else:
         block_type = 'generic'
      block_by = 1

   depend_block = False
   depend_task  = False
   if i < lenInputs - 1:
      next_name = inputs[i+1].name()
      next_trange = inputs[i+1].parm('trange').eval()
      if next_trange == 2:
         if not has_depend_mask: depend_mask = next_name
         depend_block = True
      else:
         if not has_depend_mask: depend_mask = next_name
         depend_task = True

   block = af.Block( name, block_type)
   block.setWorkingDirectory( asset)
   block.setCommand( block_cmd)
   if preview != '': block.setFiles( preview)
   block.setNumeric( block_start, block_end, block_by)
   if depend_block: block.setDependMask( depend_mask)
   if depend_task: block.setTasksDependMask( depend_mask)
   if block_capacity > 0: block.setCapacity( block_capacity)
   if block_capmin != -1 or block_capmax != -1: block.setVariableCapacity( block_capmin, block_capmax)
   job.blocks.append( block)

hou.hscript('mwrite -n %s' % tmphip)
job.send()

