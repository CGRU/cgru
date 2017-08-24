# !/usr/bin/env hython
# -*- coding: utf-8 -*-

import os
import sys
from optparse import OptionParser

import parsers.hbatch

import hou

parser = OptionParser(usage="usage: %prog [options] hip_name rop_name", version="%prog 1.0")
parser.add_option('-s', '--start',         dest='start',         type='float',  help='Start frame number.')
parser.add_option('-e', '--end',           dest='end',           type='float',  help='End frame number.')
parser.add_option('-b', '--by',            dest='by',            type='float',  help='Frame increment.')
parser.add_option('-t', '--take',          dest='take',          type='string', help='Take name.')
parser.add_option(      '--wedge',         dest='wedge',         type='string', help='Wedge path.')
parser.add_option(      '--wedgenum',      dest='wedgenum',      type='int',    help='Wedge number.')
parser.add_option('-o', '--out',           dest='output',        type='string', help='Output file.')
parser.add_option(      '--diskfile',      dest='diskfile',      type='string', help='Set to generate this file only.')
parser.add_option(      '--numcpus',       dest='numcpus',       type='int',    help='Number of CPUs.')
parser.add_option(      '--ds_node',       dest='ds_node',       type='string', help='Distribute simulation control node.')
parser.add_option(      '--ds_address',    dest='ds_address',    type='string', help='Distribute simulation tracker address.')
parser.add_option(      '--ds_port',       dest='ds_port',       type='int',    help='Distribute simulation tracker port.')
parser.add_option(      '--ds_slice',      dest='ds_slice',      type='int',    help='Distribute simulation slice number.')
parser.add_option('-i', '--ignore_inputs', dest='ignore_inputs', action='store_true', default=False, help='Ignore inputs')

options, args = parser.parse_args()

rop = []
if len(args) < 2:
    parser.error('At least one of mandatory rop_name or hip_name argument is missed.')
elif len(args) > 2:
    parser.error('Too many arguments provided.')
else:
    hip = args[0]
    rop = args[1]


start        = options.start
end          = options.end
by           = options.by
take         = options.take
wedge        = options.wedge
wedgenum     = options.wedgenum
diskfile     = options.diskfile
numcpus      = options.numcpus
output       = options.output
ignoreInputs = options.ignore_inputs


# Loading HIP file, and force HIP variable:
force_hip = True  # not sure, we need to force HIP variable.
# May be this code is obsolete
if force_hip:
    envhip = os.path.abspath(hip)
    envhip = os.path.dirname(envhip)

    if os.name != 'nt':
        envhip = envhip.replace('\\\\', '/')  # for nt //server/share/path is
        # the only one way

    envhip = envhip.replace('\\', '/')  # houdini always use unix-like path
    os.environ['HIP'] = envhip
    hou.allowEnvironmentToOverwriteVariable('HIP', True)
    hou.hscript('set HIP=' + envhip)

# Note that we ignore all load warnings.
try:
    hou.hipFile.load(hip)
except hou.LoadWarning:
    pass
# hou.hipFile.load( hip, True)

if force_hip:
    hou.hscript('set HIPNAME=%s' % os.path.basename(hip))
    hou.hscript('set HIP=%s' % envhip)
    print('HIP set to "%s"' % envhip)

# Establish ROP to be used
if rop[0] != '/':
    rop = '/out/' + rop

ropnode = hou.node(rop)

if ropnode is None:
    raise hou.InvalidNodeName(rop + ' rop node wasn`t found')

# Trying to set ROP to block until render comletes
# block = ropnode.parm('soho_foreground')
# if block != None:
# value = block.eval()
# if value != None:
# if value != 1:
# try:
# block.set( 1)
# except:
# print 'Failed to set node blocking.'

print wedge, wedgenum

if wedge:
    print 'working with wedges'
    wedgenode = hou.node(wedge)

    # below code use use script from wedge node definition
    allwedge, stashedparms, errormsg = wedgenode.hdaModule().getwedges(wedgenode)

    # apply wedge
    wedgenode.hdaModule().setenvvariable("WEDGENUM", str(wedgenum))
    wedgenode.hdaModule().applyspecificwedge(wedgenode, allwedge[wedgenum])


drivertypename = ropnode.type().name()

if drivertypename == 'ifd':

    # Set ROP to generate diskfile:
    if diskfile is not None:
        ropnode.parm('soho_outputmode').set(1)
        ropnode.parm('soho_diskfile').set(diskfile)
    else:
        # Trying to variate number of CPUs if needed:
        if numcpus is not None:
            paramcmd = ropnode.parm('soho_pipecmd')
            if paramcmd is not None:
                command = paramcmd.eval()
                if command is not None:
                    print('Trying to set mantra threads to %s' % numcpus)
                    command = command.replace(
                        'mantra',
                        'mantra -j %s' % numcpus,
                        1
                    )
                    try:
                        paramcmd.set(command)
                        print('   Command changed to:')
                        print(command)
                    except:
                        print('Failed, number of mantra theads not changed.')

        # Trying to set ROP verbose level:
        verbose = ropnode.parm('vm_verbose')
        if verbose is not None:
            value = verbose.eval()
            if value is not None:
                if value < 1:
                    print('Trying to set "Verbose Level" to 1 on mantra')
                    try:
                        verbose.set(1)
                    except:
                        print('Failed, frame verbose not available.')

        # Trying to set ROP to output progress
        progress = ropnode.parm('vm_alfprogress')
        if progress is not None:
            value = progress.eval()
            if value is not None:
                if value != 1:
                    print('Trying to set "Alfred Style Progress" on mantra')
                    try:
                        progress.set(1)
                    except:
                        print('Failed, frame progress not available.')

elif drivertypename == 'rib':

    # Trying to set ROP to output progress
    progress = ropnode.parm('soho_pipecmd')
    if progress is not None:
        command = progress.eval()
        if command is not None:
            if command.find('-progress') == -1:
                print('Trying to set "-progress" on prman')
                command = command.replace('prman', 'prman -progress', 1)
                try:
                    progress.set(command)
                except:
                    print('Failed, frame progress not available.')

#elif drivertypename == "wedge":
#
#    ropnode.parm('wrange').set(1)
#    ropnode.parm('wedgenum').set(start)
#    driverpath = ropnode.parm('driver').eval()
#    driver = ropnode.node(driverpath)
#    if driver:
#        for parmName in ["vm_alfprogress", "alfprogress"]:
#            progress = driver.parm(parmName)
#            if progress:
#                progress.set(1)


elif drivertypename == "arnold":
    # Trying to set ROP to output progress
    progress = ropnode.parm('ar_log_verbosity')
    if progress is not None:
        value = progress.eval()
        if value == 'warnings':
            print('Trying to set "Right Output Style Progress" on arnold')
            try:
                progress.set('info')
            except:
                print('Failed, frame progress not available.')

elif drivertypename == "alembic":
    ropnode.parm('lpostframe').set('python')
    expr = r'''import sys
f = hou.parmTuple('f').eval()
percent = int(100*(hou.frame()-f[0])/(f[1]-f[0]))

out = 'ABC_PROGRESS ' + str(percent) + '%\n'

sys.stdout.write(out)
sys.stdout.flush()'''
    ropnode.parm('postframe').set(expr)

#
# Distribute simulation:
#
ds_node = hou.node( options.ds_node)
if ds_node is not None:
    tracker_address = os.getenv('TRACKER_ADDRESS', options.ds_address )
    tracker_port = int( os.getenv('TRACKER_PORT', options.ds_port ))
    sim_slice = options.ds_slice

    print('Setting distributed simulation parameters:')
    print('Tracker: %s:%d' % ( tracker_address, tracker_port))
    print('Simulation slice = %d' % sim_slice)

    ds_node.parm('visaddress').set( tracker_address)
    ds_node.parm('port'   ).set( tracker_port   )
    # ds_node.parm('slice'  ).set( sim_slice      ) default value is $SLICE and can't be set with this

    # Setting $SLICE environment variable
    sli_var = "SLICE"

    try:
        hou.allowEnvironmentVariableToOverwriteVariable(sli_var, True)
    except AttributeError:
        # should be Houdini 12
        hou.allowEnvironmentToOverwriteVariable(sli_var, True)

    hscript_command = "set -g %s = '%s'" % (sli_var, sim_slice)
    hou.hscript(str(hscript_command))

    print('ACTIVITY: %s:%d/%d' % (tracker_address, tracker_port, sim_slice))

# Set take, if specified:
if take is not None and len(take) > 0:
    hou.hscript('takeset ' + take)


# If end wasn't specified, render single frame
if end is None:
    end = start

# If by wasn't specified, render every frame
if by is None:
    by = 1

render_output = None
if output is not None and len(output) > 0:
    render_output = output

frame = start
while frame <= end:
    render_range = (frame, frame, by)
    print(parsers.hbatch.keyframe + str(frame))
    sys.stdout.flush()
    ropnode.render(
        frame_range=render_range,
        output_file=render_output,
        method=hou.renderMethod.FrameByFrame,
        ignore_inputs=ignoreInputs
    )
    frame += by
