import soho
from soho import SohoParm
import hou
import os
import re
import time

genParms = {
    'trange'                     : SohoParm('trange'                    ,'int'   , [0],          False),
    'f'                          : SohoParm('f'                         ,'int'   , [1,1,1],      False),
    'hdriver'                    : SohoParm('hdriver'                   ,'string', [''],         False),
    'fpr'                        : SohoParm('fpr'                       ,'int'   , [1],          False),
    'jobname'                    : SohoParm('jobname'                   ,'string', [''],         False),
    'ignore_inputs'              : SohoParm('ignore_inputs'             ,'int'   , [0],          False),
    'take'                       : SohoParm('take'                      ,'string', ['_current_'],False),

    'gen_name'                   : SohoParm('object:name'               ,'string', [''], False, key='gen_name'),
    'now'                        : SohoParm('state:time'                ,'real'  , [0],  False, key='now'),
    'fps'                        : SohoParm('state:fps'                 ,'real'  , [24], False, key='fps'),
    'hip'                        : SohoParm('$HIP'                      ,'string', [''], False, key='hip'),
    'hipname'                    : SohoParm('$HIPNAME'                  ,'string', [''], False, key='hipname'),

    'enable_extended_parameters' : SohoParm('enable_extended_parameters','int'   , [0]    , False),
    'depend_mask'                : SohoParm('depend_mask'               ,'string', ['']   , False),
    'priority'                   : SohoParm('priority'                  ,'int'   , [99]   , False),
    'maximum_hosts'              : SohoParm('maximum_hosts'             ,'int'   , [-1]   , False),
    'hosts_mask'                 : SohoParm('hosts_mask'                ,'string', ['']   , False),
    'hosts_mask_exclude'         : SohoParm('hosts_mask_exclude'        ,'string', ['']   , False),
    'start_paused'               : SohoParm('start_paused'              ,'int'   , [0]    , False),
}
plist = soho.evaluate(genParms)

hdriver        = plist['hdriver'].Value[0]
trange         = plist['trange'].Value[0]
f              = plist['f'].Value
gen_name       = plist['gen_name'].Value[0]
fpr            = plist['fpr'].Value[0]
jobname        = plist['jobname'].Value[0]
ignore_inputs  = plist['ignore_inputs'].Value[0]
take           = plist['take'].Value[0]
FPS            = plist['fps'].Value[0]
now            = plist['now'].Value[0]
hip            = plist['hip'].Value[0]
hipname        = plist['hipname'].Value[0]

start_paused               = plist['start_paused'].Value[0]
enable_extended_parameters = plist['enable_extended_parameters'].Value[0]
depend_mask                = plist['depend_mask'].Value[0]
priority                   = plist['priority'].Value[0]
maximum_hosts              = plist['maximum_hosts'].Value[0]
hosts_mask                 = plist['hosts_mask'].Value[0]
hosts_mask_exclude         = plist['hosts_mask_exclude'].Value[0]


# Initialize soho
soho.initialize(now, '')
soho.lockObjects(now)

# First, we find the name of the output driver being called
rop = soho.getOutputDriver()
gen = hou.node(gen_name)

# Find the output driver to process
driver = gen.node(hdriver)
if not driver:
    soho.error('Can\'t find %s for gendalf processing' % hdriver)
if not isinstance(driver,hou.RopNode):
    soho.error('%s is not a ROP node' % hdriver)
tr = driver.parm('trange')
if tr!=None and tr.eval()==0:
   sf = driver.parm('soho_foreground')
   if sf==None or sf.eval()==0:
      soho.error('Set "Block Until Render Complete" on %s node' % hdriver)

if not os.path.isdir(hip):
    soho.error('Unable to find spool directory "%"' % hip)

# Get the frame range
# Ensure the frame range is at least three values
if len(f) == 1:
    f.append(f[0])
if len(f) == 2:
    f.append(1)
if trange == 0:
    # No frame range specified, just render the current frame
    f[0] = hou.frame()
    f[1] = f[0]
    f[2] = 1

start,end,by = f

ftime = time.time()
tmphip = hip + '/' + jobname + time.strftime('.%m%d-%H%M%S-') + str(ftime - int(ftime))[2:5] + ".hip"
# use mwrite, because hou.hipFile.save(tmphip)
# changes current scene file name to tmphip, at least in version 9.1.115
hou.hscript('mwrite -n %s' % tmphip)

jobcmd = 'hrender_af'
if ignore_inputs: jobcmd += ' -i'
jobcmd += ' -s %%1 -e %%2 -b %(by)d -t %(take)s %(tmphip)s %(hdriver)s' % vars()

blocktype = 'hbatch'
drivertypename = driver.type().name()
if drivertypename == 'ifd':
   blocktype = 'hbatch_mantra'
elif drivertypename == 'rib':
   blocktype = 'hbatch_prman'
elif drivertypename == 'afanasy':
   soho.error('Render "afanasy" ROP will cycle process.')
elif drivertypename == 'af_cmd_sender':
   soho.error('ROP "af_cmd_sender" can send commands itself.')
elif drivertypename == 'af_multiply_sender':
   soho.error('ROP "af_multiply_sender" sends job itself.')

asset = os.getenv('ASSET', os.getcwd())

cmd = 'afjob %(tmphip)s %(start)d %(end)d -fpr %(fpr)d -pwd %(asset)s -node %(hdriver)s -take %(take)s -name %(jobname)s -deletescene' % vars()

print cmd

os.system( cmd)
