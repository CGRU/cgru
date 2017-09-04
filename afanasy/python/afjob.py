#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
import sys
import json

import af
import afcommon
import cgruutils
import services.service


def error_exit(error_str=None):
    if error_str is not None:
        print(error_str)
    sys.stdout.flush()
    sys.exit(1)


def usage_exit():
    print("""
examples:

afjob path/scene.shk 1 100

afjob path/scene.hip 1 100 -fpt 3 -pwd projects/test -node /out/mantra1 -take back -name my_job

arguments:

path/scene.shk          - (R) Scene, which file extension determinate run command and task type
1                       - (R) First frame to render
100                     - (R) Last frame to render
-by 1                   - Frames increment, default = 1
-fpt 1                  - Frames per task, default = 1
-seq 1                  - Frames sequential task solving, default = 1
-pwd                    - Working directory, if not set current will be used.
-name                   - Job name, if not set scene name will be used.
-proj                   - Project ( maya project ).
-node                   - Node to render ( houdini driver, nuke write, max camera )
-type                   - Service type
-take                   - Take to use ( houdini take, xsi pass, max batch )
-ignoreinputs           - not to render input nodes ( houdini ignore inputs ROP parameter )
-tempscene              - copy scene to temporary file to render
-deletescene            - delete scene when job deleted
-pause                  - start job paused ( offline afanasy state )
-os                     - OS needed mask, "any" to render on any platform
-hostsmask              - job render hosts mask
-hostsexcl              - job render hosts to exclude mask
-lifetime               - job life time in seconds
-maxruntasks            - maximum number of hosts to use
-maxruntime             - maximum run time for task in seconds
-maxtasksperhost        - maximum number of tasks per host
-priority               - job priority
-capacity               - tasks capacity
-capmin                 - tasks minimum capacity coefficient
-capmax                 - tasks maximum capacity coefficient
-depmask                - wait untill other jobs of the same user, satisfying this mask
-depglbl                - wait untill other jobs of any user, satisfying this mask
-output                 - override output filename
-images                 - images to preview (img.%04d.jpg)
-exec                   - customize command executable.
-extrargs               - add to command extra arguments.
-simulate               - enable simulation
-noscript               - enable the use of the render-scripts (nuke & c4d)
-script                 - set a startup script
-mname                  - Movie name to encode from images
-mcodec                 - Movie codec
-mres                   - Movie resolution
-varirender attr start step count - variate parameter
-V                      - verbose
(R)                     - REQUIRED arguments

""")
    error_exit()


def integer(string):
    """Try to convert the given to integer, if not possible exit

    :param str string: A str value to be converted to integer
    :return:
    """
    number = ''
    try:
        number = int(string)
    except ValueError:
        error_exit(str(sys.exc_info()[1]))
    return number


if __name__ == '__main__':
    argsv = sys.argv
    argsl = len(argsv)

    if argsl < 4:
        usage_exit()

    scene = argsv[1]
    ext = scene.rfind('.')
    if ext == -1:
        ext = ''
    else:
        ext = scene[ext + 1:]
    name = os.path.basename(scene)

    #
    # initial arguments values

    s = integer(argsv[2])
    e = integer(argsv[3])
    fpt = 1
    by = 1
    seq = 1
    pwd = os.getenv('PWD', os.getcwd())
    file_ = ''
    proj = ''
    node = ''
    ignoreinputs = False
    take = ''
    deletescene = False
    tempscene = False
    startpaused = False
    hostsmask = ''
    hostsexcl = ''
    maxruntime = 0
    maxruntasks = -1
    maxtasksperhost = -1
    priority = -1
    capacity = -1
    capmin = -1
    capmax = -1
    dependmask = ''
    dependglobal = ''
    output = ''
    images = ''
    extrargs = ''
    blocktype = ''
    blockparser = ''
    platform = ''
    varirender = False
    simulate = False
    noscript = False
    script = ''
    mname = ''
    mcodec = ''
    mres = ''
    lifetime = -1

    Verbose = False
    cmd = None
    cmds = []
    blocknames = []
    blockparsers = []

    #
    # checking some critical arguments values
    if e < s:
        error_exit('End frame should be less or equal to start frame.')

    #
    # getting arguments
    for i in range(argsl):
        arg = argsv[i]

        if arg == '-by':
            i += 1
            if i == argsl:
                break
            by = integer(argsv[i])
            continue

        if arg == '-fpr':
            print('"-fpr" (frame per render) is obsolete, use "-fpt" '
                  '(frame per task) instead.')
            arg = '-fpt'

        if arg == '-fpt':
            i += 1
            if i == argsl:
                break
            fpt = integer(argsv[i])
            continue

        if arg == '-seq':
            i += 1
            if i == argsl:
                break
            seq = integer(argsv[i])
            continue

        if arg == '-pwd':
            i += 1
            if i == argsl:
                break
            pwd = argsv[i]
            continue

        if arg == '-exec':
            i += 1
            if i == argsl:
                break
            cmd = '%s' % argsv[i]
            continue

        if arg == '-file':
            i += 1
            if i == argsl:
                break
            file_ = argsv[i]
            continue

        if arg == '-name':
            i += 1
            if i == argsl:
                break
            name = argsv[i]
            continue

        if arg == '-proj':
            i += 1
            if i == argsl:
                break
            proj = argsv[i]
            continue

        if arg == '-node':
            i += 1
            if i == argsl:
                break
            node = argsv[i]
            continue

        if arg == '-extrargs':
            i += 1
            if i == argsl:
                break
            extrargs = argsv[i]
            continue

        if arg == '-take':
            i += 1
            if i == argsl:
                break
            take = argsv[i]
            continue

        if arg == '-deletescene':
            deletescene = True
            continue

        if arg == '-tempscene':
            tempscene = True
            continue

        if arg == '-ignoreinputs':
            ignoreinputs = True
            continue

        if arg == '-pause':
            startpaused = True

        if arg == '-hostsmask':
            i += 1
            if i == argsl:
                break
            hostsmask = argsv[i]
            continue

        if arg == '-hostsexcl':
            i += 1
            if i == argsl:
                break
            hostsexcl = argsv[i]
            continue

        if arg == '-maxhosts':
            print('"-maxhosts" (maximum hosts) is absolete, '
                  'use "-maxruntasks" (maximum running tasks) instead.')
            arg = '-maxruntasks'

        if arg == '-maxruntasks':
            i += 1
            if i == argsl:
                break
            maxruntasks = integer(argsv[i])
            continue

        if arg == '-maxruntime':
            i += 1
            if i == argsl:
                break
            maxruntime = integer(argsv[i])
            continue

        if arg == '-maxtasksperhost':
            i += 1
            if i == argsl:
                break
            maxtasksperhost = integer(argsv[i])

        if arg == '-priority':
            i += 1
            if i == argsl:
                break
            priority = integer(argsv[i])
            continue

        if arg == '-capacity':
            i += 1
            if i == argsl:
                break
            capacity = int(argsv[i])
            continue

        if arg == '-capmin':
            i += 1
            if i == argsl:
                break
            capmin = int(argsv[i])
            continue

        if arg == '-capmax':
            i += 1
            if i == argsl:
                break
            capmax = int(argsv[i])
            continue

        if arg == '-depmask':
            i += 1
            if i == argsl:
                break
            dependmask = argsv[i]
            continue

        if arg == '-depglbl':
            i += 1
            if i == argsl:
                break
            dependglobal = argsv[i]
            continue

        if arg == '-images':
            i += 1
            if i == argsl:
                break
            images = argsv[i]
            continue

        if arg == '-output':
            i += 1
            if i == argsl:
                break
            output = argsv[i]
            continue

        if arg == '-type':
            i += 1
            if i == argsl:
                break
            blocktype = argsv[i]
            continue

        if arg == '-os':
            i += 1
            if i == argsl:
                break
            platform = argsv[i]
            continue

        if arg == '-varirender':
            i += 1
            if i == argsl:
                break
            varirender_attr = argsv[i]
            i += 1
            if i == argsl:
                break
            varirender_start = integer(argsv[i])
            i += 1
            if i == argsl:
                break
            varirender_step = integer(argsv[i])
            i += 1
            if i == argsl:
                break
            varirender_count = integer(argsv[i])
            varirender = True
            continue

        if arg == '-simulate':
            simulate = True
            continue

        if arg == '-noscript':
            noscript = True
            continue

        if arg == '-script':
            i += 1
            if i == argsl:
                break
            script = argsv[i]
            continue

        if arg == '-mname':
            i += 1
            if i == argsl:
                break
            mname = argsv[i]
            continue

        if arg == '-mcodec':
            i += 1
            if i == argsl:
                break
            mcodec = argsv[i]
            continue

        if arg == '-mres':
            i += 1
            if i == argsl:
                break
            mres = argsv[i]
            continue

        if arg == '-V':
            Verbose = True
            continue

        if arg == '-lifetime':
            i += 1
            if i == argsl:
                break
            lifetime = argsv[i]
            continue
    #
    # command construction:
    cmdextension = os.getenv('AF_CMDEXTENSION', '')

    # Check some parameters:
    if fpt < 1:
        print('fpt - frames per task - must be > 0 ( setting to 1)')
        fpt = 1

    if by < 1:
        print('by - frames step (by frame) - must be > 0 ( setting to 1)')
        by = 1

    if tempscene:
        scene = cgruutils.copyJobFile(scene, name, ext)
        if scene == '':
            sys.exit(1)

    # Initialize general parameters:
    blockname = node

    # Shake:
    if ext == 'shk':
        scenetype = 'shake'
        if cmd is None:
            cmd = 'shake' + cmdextension
        cmd += ' -exec ' + scene + ' ' + extrargs + ' -vv -t @#@-@#@'

    # Blender:
    if ext == 'blend':
        scenetype = 'blender'
        blockparser = 'blender_render'
        if cmd is None:
            cmd = 'blender' + cmdextension
        cmd += ' -b "%s"' % scene
        if extrargs != '':
            cmd += ' ' + extrargs
            if extrargs.find('CYCLES') > 0:
                blockparser = 'blender_cycles'
        if output != '':
            cmd += ' -o "%s"' % output
            if images == '':
                images = output
                pos = images.find('#')
                if pos > 0:
                    images = images[:pos] + '@' + images[pos:]
                pos = images.rfind('#')
                if pos > 0:
                    images = images[:pos + 1] + '@' + images[pos + 1:]
        cmd += ' -s @#@ -e @#@ -j %d -a' % by
        blockname = blockparser

    # Natron:
    elif ext == 'ntp':
        scenetype = 'natron'
        if cmd is None:
            cmd = 'natron' + cmdextension
        if extrargs != '':
            cmd += ' ' + extrargs
        cmd += ' -b -w "%s"' % node

        if output != '':
            cmd += ' "%s"' % output
            if images == '':
                images = output
                pos = images.find('#')
                if pos > 0:
                    images = images[:pos] + '@' + images[pos:]
                pos = images.rfind('#')
                if pos > 0:
                    images = images[:pos + 1] + '@' + images[pos + 1:]

        cmd += ' @#@-@#@'
        cmd += ' "%s"' % scene

    # Nuke:
    elif ext == 'nk':
        scenetype = 'nuke'
        if cmd is None:
            cmd = 'nuke' + cmdextension
        # cmd += ' -i'
        if script:
            cmd += ' -t %s' % script
        elif not noscript:
            # That the Nuke-Render-Script runs which rewrites the Script
            nukerenderscript = os.environ['CGRU_LOCATION']
            nukerenderscript = os.path.join(nukerenderscript, 'plugins')
            nukerenderscript = os.path.join(nukerenderscript, 'nuke')
            nukerenderscript = os.path.join(nukerenderscript, 'render.py')
            cmd += ' -t %s' % nukerenderscript

        cmd += ' -F @#@-@#@x' + str(by)
        if capmin != -1 or capmax != -1:
            cmd += ' -m ' + services.service.str_capacity

        if node != '':
            cmd += ' -X %s' % node

        cmd += ' -x "%s"' % scene

        if extrargs != '':
            cmd += ' ' + extrargs

    # Houdini:
    elif ext == 'hip':
        if node == '':
            error_exit('no houdini driver to render specified')
        scenetype = 'hbatch_mantra'
        if cmd is None:
            cmd = 'hrender_af' + cmdextension
        if capmin != -1 or capmax != -1:
            cmd += ' --numcpus ' + services.service.str_capacity

        if ignoreinputs:
            cmd += ' -i'

        cmd += ' -s @#@ -e @#@ --by %d' % by

        if take != '':
            cmd += ' -t "%s"' % take

        if extrargs != '':
            cmd += ' ' + extrargs

        cmd += ' "%s" "%s"' % (scene, node)

    # Mantra:
    elif ext == 'ifd':
        scenetype = 'mantra'
        if cmd is None:
            cmd = 'mantra' + cmdextension

        if capmin != -1 or capmax != -1:
            cmd += ' -j ' + services.service.str_capacity

        if extrargs != '':
            cmd += ' ' + extrargs

        cmd += ' -V a -f "%s"' % scene

    # Maya (3Delight and Mental Ray):
    elif ext in ['ma', 'mb']:
        if blocktype == 'maya_mental':
            scenetype = 'maya_mental'
        elif blocktype == 'maya_delight':
            scenetype = 'maya_delight'
        elif blocktype == 'maya_arnold':
            scenetype = 'maya_arnold'
        elif blocktype == 'maya_redshift':
            scenetype = 'maya_redshift'
        else:
            blocktype = 'maya'
            scenetype = 'maya'
        # cmd = 'mayabatch' + cmdextension + ' -file "' + scene + '" -command "afanasyBatch(@#@,@#@,1,1);quit -f;"'

        if cmd is None:
            cmd = 'mayarender%s' % cmdextension

        if scenetype == 'maya_mental':
            cmd += ' -r mr'

        if scenetype == 'maya_delight':
            cmd += ' -r 3delight'

        if scenetype == 'maya':
            cmd += ' -r file'

        if scenetype != 'maya_delight':
            cmd += ' -s @#@ -e @#@ -b %d' % by
        else:
            cmd += ' -an 1 -s @#@ -e @#@ -inc %d' % by

        if node != '':
            cmd += ' -cam "%s"' % node

        if take != '':
            if scenetype == 'maya_delight':
                cmd += ' -rp "%s"' % take
            else:
                cmd += ' -rl "%s"' % take
        if images != '':
            images = images.split(',')
            if len(images) > 1:
                images = afcommon.patternFromPaths(images[0], images[1])
            else:
                images = afcommon.patternFromFile(images[0])

        if proj:
            cmd += ' -proj "%s"' % os.path.normpath(proj)
        # else:
        #     if pwd != '':
        #         cmd += ' -proj "%s"' % os.path.normpath(pwd)
        #     else:
        #         cmd += ' -proj "%s"' % os.path.normpath(os.path.dirname(scene))

        if output != '':
            cmd += ' -rd "%s"' % os.path.normpath(output)

        if scenetype == 'maya_mental':
            cmd += ' -art -v 5'

        if extrargs != '':
            cmd += ' ' + extrargs

        cmd += ' "%s"' % scene

    # XSI:
    elif ext == 'scn':
        scenetype = 'xsi'
        if cmd is None:
            cmd = 'xsibatch' + cmdextension

        if script:
            cmd += ' -script %s' % script
        elif not noscript:
            xsirenderscript = os.environ['CGRU_LOCATION']
            xsirenderscript = os.path.join(xsirenderscript, 'plugins')
            xsirenderscript = os.path.join(xsirenderscript, 'xsi')
            xsirenderscript = os.path.join(xsirenderscript, 'afrender.py')
            cmd += ' -script %s' % xsirenderscript

        cmd += ' -lang Python -main afRender -args'
        cmd += ' -scene "%s"' % scene
        cmd += ' -start @#@ -end @#@ -step ' + str(by)
        cmd += ' -simulate'

        if extrargs != '':
            cmd += ' ' + extrargs

        if simulate:
            cmd += ' 1'
        else:
            cmd += ' 0'

        if take != '':
            cmd += ' -renderPass ' + take

        if varirender:
            cmd += ' -attr %s -value ' % varirender_attr
            value = varirender_start

            for i in range(0, varirender_count):
                cmds.append(cmd + str(value))
                blocknames.append('variant[%d]' % value)
                value += varirender_step

            cmd = ''

    # 3D MAX:
    elif ext == 'max':
        scenetype = 'max'

        if cmd is None:
            cmd = '3dsmaxcmd' + cmdextension

        cmd += ' "' + scene + '" -start:@#@ -end:@#@ -nthFrame:' + str(
            by) + ' -v:5 -gammaCorrection:1 -continueOnError -showRFW:0'

        if node != '':
            cmd += ' -cam:"%s"' % node

        if take != '':
            cmd += ' -batchrender'
            if take != 'all':
                cmd += ':"%s"' % take

            if blockname != '':
                blockname = take + ' ' + blockname
            else:
                blockname = take

        if output != '':
            cmd += ' -o:"%s"' % output
            images = output

        if extrargs != '':
            cmd += ' ' + extrargs

    # After FX:
    elif ext == 'aep':
        scenetype = 'afterfx'

        if cmd is None:
            cmd = 'aerender' + cmdextension
        cmd += ' -project "%s"' % scene
        cmd += ' -s @#@ -e @#@ -i %d' % by

        if node != '':
            cmd += ' -comp "%s"' % node

        if take != '':
            cmd += ' -RStemplate "%s"' % take

        images = os.path.join(
            os.path.dirname(images),
            os.path.basename(images).replace('[', '@').replace(']', '@')
        )

        if output != '':
            output = os.path.join(os.getcwd(), output)
            cmd += ' -output "%s"' % output
            images = os.path.join(
                os.path.dirname(output),
                os.path.basename(output).replace('[', '@').replace(']', '@')
            )

        if extrargs != '':
            cmd += ' ' + extrargs

    # C4D:
    elif ext == 'c4d':
        scenetype = 'c4d'

        if cmd is None:
            cmd = 'c4d' + cmdextension

        if not noscript:
            cmd = 'c4d' + cmdextension

        # Here I get the information in there that the normal c4d and not the
        # special c4drender command should get used (which copies all the stuff
        # locally). As "c4drender" should be the special case this defaults to
        # "c4d" now and takes the argument "copy_scene_files" to invoke
        # "c4drender"
        if extrargs == 'copy_scene_files':
            cmd = 'c4drender' + cmdextension

        cmd += ' -nogui -render "%s" -frame @#@ @#@ %s' % (scene, by)

        if output != '':
            cmd += ' -oimage "%s"' % output
            images = output

    # VRay:
    elif ext == 'vrscene':
        scenetype = 'vray'

        if cmd is None:
            cmd = 'vray' + cmdextension

        cmd += ' -sceneFile="%s"' % scene

        if re.search(r'@#+@', scene) is None:
            cmd += ' -frames=@#@-@#@,' + str(by)

        if output != '':
            cmd += ' -imgFile="%s"' % output
            images = output

        cmd += ' -display=0 -autoClose=1 -verboseLevel=4 -showProgress=2'

    # Arnold:
    elif ext == 'ass':
        scenetype = 'arnold'
        if cmd is None:
            cmd = 'kick' + cmdextension

        cmd += ' -i "%s"' % scene

        if output != '':
            cmd += ' -o "%s"' % output
            images = output

        cmd += ' -v 2 -nstdin -dw -dp -nocrashpopup'

    # Fusion:
    elif ext == 'comp':
        scenetype = 'fusion'
        if cmd is None:
            cmd = 'fusion' + cmdextension

        cmd += ' "%s"' % scene
        cmd += ' /render /start @#@ /end @#@ /step %d /quiet' % by
        cmd += ' /quietlicense /clean /quit /log %TEMP%/fusion_render.log'

    # Clarisse:
    elif ext == 'render':
        scenetype = 'clarisse'
        if cmd is None:
            cmd = 'clarisse_node' + cmdextension

        cmd += ' "%s"' % scene
        if node != '':
            cmd += ' -image "%s"' % node
        cmd += ' -start_frame @#@ -end_frame @#@ -frame_step %d' % by
        if output != '':
            cmd += ' -output "%s"' % output
            images = output
        if take != '':
            cmd += ' -format "%s"' % take
        cmd += ' -log_width 0'

    # simple generic:
    else:
        scenetype = 'generic'

        if output != '' and images == '':
            images = output

        if cmd is None:
            cmd = scene

        if extrargs != '':
            cmd += ' ' + extrargs

        cmd += ' @#@ @#@'

    #
    # Creating a Job:
    #

    # Create a Block(s) af first:
    blocks = []
    if blockname == '':
        blockname = scenetype

    if blocktype == '':
        blocktype = scenetype

    if len(cmds) == 0:
        cmds.append(cmd)
        blocknames.append(blockname)

    if len(blockparsers) == 0 and blockparser != '':
        blockparsers.append(blockparser)

    for i, cmd in enumerate(cmds):
        block = af.Block(blocknames[i], blocktype)

        if len(blockparsers):
            block.setParser(blockparsers[i])

        block.setWorkingDirectory(pwd)
        block.setNumeric(s, e, fpt, by)
        if seq != 1:
            block.setSequential( seq)

        if scenetype == 'max':
            block.setCommand(cmd, False, False)
        else:
            block.setCommand(cmd)

        block.setCapacity(capacity)
        block.setVariableCapacity(capmin, capmax)
        if maxruntime != 0:
            block.setTasksMaxRunTime(maxruntime)

        if images != '':
            block.setFiles(
                afcommon.patternFromDigits(
                    afcommon.patternFromStdC(images)
                ).split(';')
            )

        blocks.append(block)

    if mname != '' and images != '':
        block = af.Block(mname + '-movie', 'movgen')
        blocks.append(block)
        block.setWorkingDirectory(pwd)
        block.setDependMask(blocknames[-1])
        task = af.Task(mname)
        block.tasks.append(task)
        cmd = os.getenv('CGRU_LOCATION')
        cmd = os.path.join(cmd, 'utilities')
        cmd = os.path.join(cmd, 'moviemaker')
        cmd = os.path.join(cmd, 'makemovie.py')
        cmd = 'python "%s"' % cmd
        if mcodec != '':
            cmd += ' --codec "%s"' % mcodec

        if mres != '':
            cmd += ' -r "%s"' % mres

        cmd += ' "%s"' % images.replace('@#', '#').replace('#@', '#')

        if not os.path.isabs(mname):
            mname = os.path.join(
                os.path.dirname(
                    os.path.dirname(images)
                ),
                mname
            )

        cmd += ' "%s"' % mname
        task.setCommand(cmd)

    # Create a Job:
    job = af.Job(name)

    job.setPriority(priority)

    job.setFolder('input', os.path.dirname(scene))

    if images != '':
        job.setFolder('output', os.path.dirname(images))

    if maxruntasks != -1:
        job.setMaxRunningTasks(maxruntasks)

    if maxtasksperhost != -1:
        job.setMaxRunTasksPerHost(maxtasksperhost)

    if hostsmask != '':
        job.setHostsMask(hostsmask)

    if hostsexcl != '':
        job.setHostsMaskExclude(hostsexcl)

    if dependmask != '':
        job.setDependMask(dependmask)

    if dependglobal != '':
        job.setDependMaskGlobal(dependglobal)

    if lifetime != -1:
        job.setTimeLife(integer(lifetime))

    if deletescene:
        job.setCmdPost('deletefiles "%s"' % os.path.abspath(scene))

    if startpaused:
        job.offline()

    if platform != '':
        if platform == 'any':
            job.setNeedOS('')
        else:
            job.setNeedOS(platform)

    # Add a Block to a Job:
    job.blocks.extend(blocks)

    # Print job information:
    if Verbose:
        job.output(True)

    # Send Job to server:
    status, data = job.send()

    print(json.dumps(data))

    if not status:
        sys.exit(1)
