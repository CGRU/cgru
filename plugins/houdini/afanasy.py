# -*- coding: utf-8 -*-

import os
import re
import time

import hou

import af
import afcommon
import services.service

VERBOSE = 0


class BlockParameters:
    def __init__(self, afnode, ropnode, subblock, prefix, frame_range, for_job_only=False):
        if VERBOSE == 2:
            if ropnode:
                print('Initializing block parameters for "%s" from "%s"' %
                      (ropnode.path(), afnode.path()))
            else:
                print('Initializing command block parameters from "%s"' %
                      afnode.path())

        # Init parameters:
        self.valid = False
        self.afnode = afnode
        self.ropnode = None
        self.subblock = subblock
        self.frame_pertask = 1
        self.frame_sequential = 1
        self.prefix = prefix
        self.preview = ''
        self.name = ''
        self.service = ''
        self.parser = ''
        self.cmd = ''
        self.cmd_useprefix = True
        self.dependmask = ''
        self.fullrangedepend = False
        self.numeric = True
        self.frame_first, self.frame_last, self.frame_inc = frame_range
        self.auxargs = ''
        self.tasks_names = []
        self.tasks_cmds = []
        self.tasks_previews = []
        # Fill in this array with files to delete in a block post command.
        # Files should have a common afanasy "@#*@" pattern,
        # it will be replaced with "*" for shell.
        self.delete_files = []
        # Parameters to restore ROP changes:
        self.soho_foreground = None
        self.soho_outputmode = None

        # Get parameters:
        self.frame_pertask = int(afnode.parm('frame_pertask').eval())
        self.frame_sequential = int(afnode.parm('frame_sequential').eval())
        self.job_name = str(afnode.parm('job_name').eval())
        self.start_paused = int(afnode.parm('start_paused').eval())
        self.platform = str(afnode.parm('platform').eval())
        self.subtaskdepend = int(afnode.parm('subtaskdepend').eval())
        self.priority = -1
        self.max_runtasks = -1
        self.maxperhost = -1
        self.maxruntime = -1
        self.capacity = -1
        self.capacity_min = -1
        self.capacity_max = -1
        self.hosts_mask = ''
        self.hosts_mask_exclude = ''
        self.depend_mask = ''
        self.depend_mask_global = ''
        self.min_memory = -1
        self.preview_approval = afnode.parm('preview_approval').eval()

        if afnode.parm('enable_extended_parameters').eval():
            self.parser = self.afnode.parm('override_parser').eval()
            self.priority = int(afnode.parm('priority').eval())
            self.max_runtasks = int(afnode.parm('max_runtasks').eval())
            self.maxperhost = int(afnode.parm('maxperhost').eval())
            self.maxruntime = int(afnode.parm('maxruntime').eval())
            self.min_memory = int(afnode.parm('min_memory').eval())
            self.capacity = int(afnode.parm('capacity').eval())
            self.capacity_min = int(
                afnode.parm('capacity_coefficient1').eval())
            self.capacity_max = int(
                afnode.parm('capacity_coefficient2').eval())
            self.hosts_mask = str(afnode.parm('hosts_mask').eval())
            self.hosts_mask_exclude = str(
                afnode.parm('hosts_mask_exclude').eval())
            self.depend_mask = str(afnode.parm('depend_mask').eval())
            self.depend_mask_global = str(
                afnode.parm('depend_mask_global').eval())

        # Process frame range:
        opname = afnode.path()
        if afnode.parm('trange').eval() > 1:
            self.fullrangedepend = True

        if ropnode:
            opname = ropnode.path()
            trange = ropnode.parm('trange')

            if trange is not None:
                if int(trange.eval()) > 0:
                    if ropnode.parm('f1') is not None:
                        self.frame_first = int(ropnode.parm('f1').eval())

                    if ropnode.parm('f2') is not None:
                        self.frame_last = int(ropnode.parm('f2').eval())

                    if ropnode.parm('f3') is not None:
                        self.frame_inc = int(ropnode.parm('f3').eval())

                if int(trange.eval()) > 1:
                    self.fullrangedepend = True

        if self.frame_last < self.frame_first:
            hou.ui.displayMessage('Last frame < first frame for "%s"' % opname)
            return

        if self.frame_inc < 1:
            hou.ui.displayMessage('Frame increment < 1 for "%s"' % opname)
            return

        if self.frame_pertask < 1:
            hou.ui.displayMessage('Frames per task < 1 for "%s"' % opname)
            return

        # Process output driver type to construct a command:
        if ropnode:
            self.service = 'hbatch'

            if not isinstance(ropnode, hou.RopNode):
                hou.ui.displayMessage(
                    '"%s" is not a ROP node' % ropnode.path()
                )
                return

            self.ropnode = ropnode
            self.name = str(ropnode.name())

            if self.prefix != '':
                self.name = '%s_%s' % (self.prefix, self.name)
            # Block type and preview:
            roptype = ropnode.type().name()

            if roptype == 'ifd':
                if ropnode.node(ropnode.parm('camera').eval())==None:
                    hou.ui.displayMessage("Camera in "+ropnode.name()+" is not valid",severity = hou.severityType.Error)
                    return

                if not ropnode.parm('soho_outputmode').eval():
                    self.service = 'hbatch_mantra'

                vm_picture = ropnode.parm('vm_picture')

                if vm_picture is not None:
                    self.preview = \
                        afcommon.patternFromPaths(
                            vm_picture.evalAsStringAtFrame(self.frame_first),
                            vm_picture.evalAsStringAtFrame(self.frame_last)
                        )
            elif roptype == 'rib':
                self.service = 'hbatch_prman'

            elif roptype == 'arnold':
                if not ropnode.parm('soho_outputmode').eval():
                    self.service = 'houdinitoarnold'

                ar_picture = ropnode.parm('ar_picture')

                if ar_picture is not None:
                    self.preview = \
                        afcommon.patternFromPaths(
                            ar_picture.evalAsStringAtFrame(self.frame_first),
                            ar_picture.evalAsStringAtFrame(self.frame_last)
                        )

            elif roptype == 'alembic':
                self.numeric = False
                taskname = ropnode.name()
                taskname += ' ' + str(self.frame_first)
                taskname += '-' + str(self.frame_last)
                self.tasks_names.append(taskname)
                self.tasks_cmds.append(self.frame_first)

            # Block command:
            self.cmd = 'hrender_af'
            if afnode.parm('ignore_inputs').eval():
                self.cmd += ' -i'

            if self.capacity_min != -1 or self.capacity_max != -1:
                self.cmd += ' --numcpus ' + services.service.str_capacity

            self.cmd += ' -s @#@ -e @#@ --by %d -t "%s"' % (
                self.frame_inc, afnode.parm('take').eval()
            )

#            numWedges = computeWedge(ropnode, roptype)
#            if numWedges:
#                self.frame_first = 0
#                self.frame_last = numWedges - 1
#                self.frame_inc = 1
#                self.frame_pertask = 1
#                self.parser = "mantra"

            self.cmd += '%(auxargs)s'
            self.cmd += ' "%(hipfilename)s"'
            self.cmd += ' "%s"' % ropnode.path()

            if afnode.parm('enable_extended_parameters').eval():
                # Override service:
                override_service = self.afnode.parm('override_service').eval()
                if override_service is not None and len(override_service):
                    self.service = override_service

        else:
            # Custom command driver:
            if int(afnode.parm('cmd_mode').eval()):
                # Command:
                cmd = self.afnode.parm('cmd_cmd')
                self.cmd = afcommon.patternFromPaths(
                    cmd.evalAsStringAtFrame(self.frame_first),
                    cmd.evalAsStringAtFrame(self.frame_last)
                )

                # Name:
                self.name = self.afnode.parm('cmd_name').eval()
                if self.name is None or self.name == '':
                    self.name = self.cmd.split(' ')[0]

                # Service:
                self.service = self.afnode.parm('cmd_service').eval()
                if self.service is None or self.service == '':
                    self.service = self.cmd.split(' ')[0]
                # Parser:
                self.parser = self.afnode.parm('cmd_parser').eval()

                # Prefix:
                self.cmd_useprefix = \
                    int(self.afnode.parm('cmd_use_afcmdprefix').eval())

                # Delete files on job deletion:
                if self.afnode.parm('cmd_delete_files').eval():
                    cmd_files = self.afnode.parm('cmd_files')
                    self.delete_files.append(afcommon.patternFromPaths(
                        cmd_files.evalAsStringAtFrame(self.frame_first),
                        cmd_files.evalAsStringAtFrame(self.frame_last)
                    ))

            elif not for_job_only:
                hou.ui.displayMessage('Can\'t process "%s"' % afnode.path())
                return

        # Try to set driver foreground mode
        if ropnode:
            trange = ropnode.parm('trange')
            soho_foreground = ropnode.parm('soho_foreground')
            if trange is not None and int(trange.eval()) == 0:
                if soho_foreground is not None:
                    if soho_foreground.eval() == 0:
                        try:
                            soho_foreground.set(1)
                            self.soho_foreground = 0
                        except:  # TODO: too broad exception clause
                            hou.ui.displayMessage(
                                'Set "Block Until Render Complete" on "%s" '
                                'node' % ropnode.path()
                            )
                            return

        # Try to create output folder:
        if self.preview != '' and afnode.parm('check_output_folder').eval():
            folder = os.path.dirname(self.preview)
            if not os.path.isdir(folder):
                if hou.ui.displayMessage(folder, buttons=('Create', 'Abort'),
                                         default_choice=0, close_choice=1,
                                         title='Output Folder Does Not Exist',
                                         details=folder) == 0:
                    try:
                        os.makedirs(folder)
                    except Exception as e:
                        hou.ui.displayMessage(folder, buttons=('Abort',),
                                              default_choice=0, close_choice=1,
                                              title='Error Creating Output Folder',
                                              details=str(e))
                        return
                    if not os.path.isdir(folder):
                        hou.ui.displayMessage(folder, buttons=('Abort',),
                                              default_choice=0, close_choice=1,
                                              title='Can`t Create Output Folder',
                                              details=folder)
                        return
                else:
                    return

        self.valid = True

    def genBlock(self, hipfilename):
        if VERBOSE:
            if self.ropnode:
                print('Generating block for "%s" from "%s"' %
                      (self.ropnode.path(), self.afnode.path()))
            else:
                print('Generating command block from "%s"' %
                      (self.afnode.path()))

        auxargs = self.auxargs
        # Place hipfilename and auxargs
        cmd = self.cmd % vars()

        block = af.Block(self.name, self.service)
        block.setParser(self.parser)
        block.setCommand(cmd, self.cmd_useprefix)
        if self.preview != '':
            block.setFiles([self.preview])

        if self.numeric:
            block.setNumeric(
                self.frame_first,
                self.frame_last,
                self.frame_pertask,
                self.frame_inc
            )
        else:
            t = 0
            for cmd in self.tasks_cmds:
                task = af.Task(self.tasks_names[t])
                task.setCommand(cmd)
                if len(self.tasks_previews):
                    task.setFiles([self.tasks_previews[t]])
                block.tasks.append(task)
                t += 1
            block.setFramesPerTask(self.frame_pertask)

        block.setSequential(self.frame_sequential)

        block.setCapacity(self.capacity)
        if self.capacity_min != -1 or self.capacity_max != -1:
            block.setVariableCapacity(self.capacity_min, self.capacity_max)

        block.setTasksMaxRunTime(self.maxruntime)

        # Delete files in a block post command:
        if len(self.delete_files):
            post_cmd = 'deletefiles'
            for files in self.delete_files:
                post_cmd += ' "%s"' % re.sub('@#*@', '*', files)
            block.setCmdPost(post_cmd)

        if self.subblock:
            if self.max_runtasks > -1:
                block.setMaxRunningTasks(self.max_runtasks)
            if self.maxperhost > -1:
                block.setMaxRunTasksPerHost(self.maxperhost)
            if self.hosts_mask != '':
                block.setHostsMask(self.hosts_mask)
            if self.hosts_mask_exclude != '':
                block.setHostsMaskExclude(self.hosts_mask_exclude)
        if self.dependmask != '':
            if self.fullrangedepend:
                block.setDependMask(self.dependmask)
            else:
                block.setTasksDependMask(self.dependmask)
        if self.subtaskdepend:
            block.setDependSubTask()
        if self.min_memory > -1:
            block.setNeedMemory(self.min_memory)

        return block

    def genJob(self, blockparams):
        if VERBOSE:
            print('Generating job on "%s"' % self.job_name)

        if len(blockparams) < 1:
            print('Can`t generate job without any blocks on "%s"' %
                  self.afnode.name())
            return

        renderhip = hou.hipFile.name()

        if self.afnode.parm('render_temp_hip').eval():
            # Calculate temporary hip name:
            ftime = time.time()
            renderhip = '%s/%s%s%s.hip' % (
                os.path.dirname(renderhip),
                afcommon.filterFileName(self.job_name),
                time.strftime('.%m%d-%H%M%S-'),
                str(ftime - int(ftime))[2:5]
            )

            # use mwrite, because hou.hipFile.save(renderhip)
            # changes current scene file name to renderhip,
            # at least in version 9.1.115
            hou.hscript('mwrite -n "%s"' % renderhip)

        job = af.Job()
        job.setName(self.job_name)

        if self.start_paused:
            job.offLine()

        if self.preview_approval:
            job.setPPApproval()

        if self.platform != '':
            if self.platform == 'any':
                job.setNeedOS('')
            else:
                job.setNeedOS(self.platform)

        if self.priority != -1:
            job.setPriority(self.priority)

        if self.depend_mask != '':
            job.setDependMask(self.depend_mask)

        if self.depend_mask_global != '':
            job.setDependMaskGlobal(self.depend_mask_global)

        if self.max_runtasks > -1:
            job.setMaxRunningTasks(self.max_runtasks)

        if self.maxperhost > -1:
            job.setMaxRunTasksPerHost(self.maxperhost)

        if self.hosts_mask != '':
            job.setHostsMask(self.hosts_mask)

        if self.hosts_mask_exclude != '':
            job.setHostsMaskExclude(self.hosts_mask_exclude)

        job.setFolder('input', os.path.dirname(hou.hipFile.name()))

        images = None
        for blockparam in blockparams:
            job.blocks.append(blockparam.genBlock(renderhip))

            # Set ouput folder from the first block with images to preview:
            if images is None and blockparam.preview != '':
                images = blockparam.preview
                job.setFolder('output', os.path.dirname(images))

        if self.afnode.parm('render_temp_hip').eval():
            job.setCmdPost('deletefiles "%s"' % renderhip)

        if VERBOSE:
            job.output(True)

        job.send()

    def doPost(self):
        if self.ropnode is None:
            return
        if VERBOSE:
            print('doPost: "%s"' % self.ropnode.path())

        if self.soho_foreground is not None:
            self.ropnode.parm('soho_foreground').set(self.soho_foreground)

        if self.soho_outputmode is not None:
            self.ropnode.parm('soho_outputmode').set(self.soho_outputmode)
            self.ropnode.parm('soho_diskfile').set('')

    def addDependMask(self, dependmask):
        if dependmask is None:
            return
        if dependmask == '':
            return
        if self.dependmask:
            self.dependmask += '|' + dependmask
        else:
            self.dependmask = dependmask


def getBlockParameters(afnode, ropnode, subblock, prefix, frame_range):

    params = []

    if ropnode is not None and ropnode.type().name() == 'ifd' and afnode.parm('sep_enable').eval():
        # Case mantra separate render:

        block_generate = \
            BlockParameters(afnode, ropnode, subblock, prefix, frame_range)
        blockname = block_generate.name
        block_generate.name += '-G'

        if not block_generate.valid:
            return None

        run_rop = afnode.parm('sep_run_rop').eval()
        read_rop = afnode.parm('sep_read_rop_params').eval()
        join_render = afnode.parm('sep_join').eval()
        tile_render = afnode.parm('sep_tile').eval()
        tile_divx = afnode.parm('sep_tile_divx').eval()
        tile_divy = afnode.parm('sep_tile_divy').eval()
        use_tmp_img_folder = afnode.parm('sep_use_tmp_img_folder').eval()
        del_rop_files = afnode.parm('sep_del_rop_files').eval()

        if read_rop or run_rop:
            if not block_generate.ropnode:
                hou.ui.displayMessage(
                    'Can`t find ROP for processing "%s"' % afnode.path()
                )

            if not isinstance(ropnode, hou.RopNode):
                hou.ui.displayMessage(
                    '"%s" is not a ROP node' % block_generate.ropnode.path()
                )

        if not run_rop:
            join_render = False

        if join_render:
            tile_render = False
        else:
            if block_generate.ropnode.parm('soho_outputmode').eval() == 0:
                # Set output mode to produce ifd files:
                block_generate.ropnode.parm('soho_outputmode').set(1)
                block_generate.soho_outputmode = 0
                block_generate.ropnode.parm('soho_diskfile').set(
                    block_generate.ropnode.parm(
                        'vm_picture').unexpandedString() + '.ifd'
                )

        if read_rop:
            parm_images = ropnode.parm('vm_picture')
            parm_files  = ropnode.parm('soho_diskfile')
        else:
            parm_images = afnode.parm('sep_images')
            parm_files  = afnode.parm('sep_files')

        images = afcommon.patternFromPaths(
            parm_images.evalAsStringAtFrame( block_generate.frame_first),
            parm_images.evalAsStringAtFrame( block_generate.frame_last))

        files = afcommon.patternFromPaths(
            parm_files.evalAsStringAtFrame( block_generate.frame_first),
            parm_files.evalAsStringAtFrame( block_generate.frame_last))

        if run_rop:
            if join_render:
                block_generate.preview = images

            if not join_render:
                block_generate.service = 'hbatch'
            else:
                block_generate.service = 'hbatch_mantra'
                block_generate.cmd = block_generate.cmd.replace(
                    'hrender_af', 'hrender_separate'
                )

                if use_tmp_img_folder:
                    block_generate.cmd += ' --tmpimg'

        if not join_render:
            tiles = tile_divx * tile_divy
            block_render = BlockParameters(afnode, ropnode, subblock, prefix,
                                           frame_range)
            block_render.name = blockname + '-R'
            block_render.cmd = 'mantra'
            block_render.service = block_render.cmd
            if run_rop:
                block_render.dependmask = block_generate.name

            if tile_render or del_rop_files or use_tmp_img_folder:
                block_render.cmd = 'mantrarender '

            if del_rop_files:
                block_render.delete_files.append(files)

            if use_tmp_img_folder:
                block_render.cmd += 't'

            if tile_render:
                block_render.numeric = False
                block_render.cmd += 'c %(tile_divx)d %(tile_divy)d' % vars()
                block_render.cmd += ' @#@'
                block_render.frame_pertask = -tiles
                for frame in range(block_generate.frame_first,
                                   block_generate.frame_last + 1,
                                   block_generate.frame_inc):
                    arguments = afnode.parm(
                        'sep_render_arguments').evalAsStringAtFrame(frame)
                    arguments = arguments.replace(
                        '@FILES@', parm_files.evalAsStringAtFrame( frame))

                    for tile in range(0, tiles):
                        block_render.tasks_names.append(
                            '%d tile %d' % (frame, tile))
                        block_render.tasks_cmds.append(
                            '%d -R %s' % (tile, arguments))
            else:
                if del_rop_files or use_tmp_img_folder:
                    block_render.cmd += ' -R '
                else:
                    block_render.cmd += ' -V a '
                block_render.cmd += afcommon.patternFromPaths(
                    afnode.parm('sep_render_arguments').evalAsStringAtFrame(block_generate.frame_first),
                    afnode.parm('sep_render_arguments').evalAsStringAtFrame(block_generate.frame_last)
                ).replace('@FILES@', files)
                block_render.preview = images

        if tile_render:
            cmd = 'exrjoin %(tile_divx)d %(tile_divy)d %(images)s d' % vars()

            block_join = BlockParameters(
                afnode, ropnode, subblock, prefix, frame_range
            )

            block_join.name = blockname + '-J'
            block_join.service = 'generic'
            block_join.dependmask = block_render.name
            block_join.cmd = cmd
            block_join.cmd_useprefix = False
            block_join.preview = images

        if tile_render:
            params.append(block_join)

        if not join_render:
            params.append(block_render)

        if run_rop:
            params.append(block_generate)

    elif len(str(afnode.parm('ds_node').eval())):
        # Case distribute simulation:
        ds_node_path = str(afnode.parm('ds_node').eval())
        ds_node = hou.node(ds_node_path)
        if not ds_node:
            hou.ui.displayMessage('No such control node: "%s"' % ds_node_path)
            return
        parms = ['address', 'port', 'slice']
        for parm in parms:
            if not ds_node.parm(parm):
                hou.ui.displayMessage('Control node "%s" does not have "%s" parameter' % (ds_node_path, parm))
                return

        enable_tracker = not afnode.parm('ds_tracker_manual').eval()
        if enable_tracker:
            # Tracker block:
            par_start = getTrackerParameters(afnode, ropnode, subblock, prefix, frame_range, True)
            params.append(par_start)

        # A block for each slice:
        ds_num_slices = int(afnode.parm('ds_num_slices').eval())
        for s in range(0, ds_num_slices):
            par = BlockParameters(afnode, ropnode, subblock, prefix, frame_range)
            sim_blocks_mask = par.name + '.*'
            par.name += '-s%d' % s
            par.frame_pertask = par.frame_last - par.frame_first + 1
            if enable_tracker:
                par.addDependMask(par_start.name)
            par.fullrangedepend = True
            par.auxargs = ' --ds_node "%s"' % ds_node_path
            par.auxargs += ' --ds_address "%s"' % str(afnode.parm('ds_address').eval())
            par.auxargs += ' --ds_port %d' % int(afnode.parm('ds_port').eval())
            par.auxargs += ' --ds_slice %d' % s
            params.append(par)

        if enable_tracker:
            # Stop tracker block:
            par_stop = getTrackerParameters(afnode, ropnode, subblock, prefix, frame_range, False)
            par_stop.addDependMask(sim_blocks_mask)
            params.append(par_stop)

            # Set other block names for start tracker block.
            # As start tracker block will set other block environment
            # to specify started tracker and port.
            par_start.cmd += ' --envblocks "%s|%s"' % (sim_blocks_mask, par_stop.name)
            # On this block depend mask will be reset on tracker start:
            par_start.cmd += ' --depblocks "%s"' % sim_blocks_mask

    else:
        params.append(
            BlockParameters(afnode, ropnode, subblock, prefix, frame_range)
        )
    return params


def getTrackerParameters(i_afnode, i_ropnode, i_subblock, i_prefix, i_frame_range, i_start):
    par = BlockParameters(i_afnode, i_ropnode, i_subblock, i_prefix, i_frame_range)
    if i_prefix:
        par.name = i_prefix + '-tracker'
    else:
        par.name = 'tracker'
    par.frame_last = par.frame_first
    par.frame_pertask = 1
    par.subtaskdepend = False
    par.fullrangedepend = True
    par.capacity = int(i_afnode.parm('ds_tracker_capacity').eval())
    par.hosts_mask = str(i_afnode.parm('ds_tracker_hostmask').eval())
    par.service = str(i_afnode.parm('ds_tracker_service').eval())
    par.parser = str(i_afnode.parm('ds_tracker_parser').eval())
    if i_start:
        par.cmd = 'htracker --start'
    else:
        par.cmd = 'htracker --stop'
        par.name += '-stop'
    return par


def getJobParameters(afnode, subblock=False, frame_range=None, prefix=''):
    if VERBOSE:
        print('Getting Job Parameters from "%s":' % afnode.path())

    # Process frame range:
    if frame_range is None:
        frame_first = int(hou.frame())
        frame_last = frame_first
        frame_inc = 1
    else:
        frame_first, frame_last, frame_inc = frame_range

    trange = afnode.parm('trange')

    if int(trange.eval()) > 0:
        frame_first = int(afnode.parm('f1').eval())
        frame_last = int(afnode.parm('f2').eval())
        frame_inc = int(afnode.parm('f3').eval())

    frame_range = frame_first, frame_last, frame_inc

    params = []
    connections = []
    connections.extend(afnode.inputs())
    nodes = []

    for node in connections:
        if node is not None:
            nodes.append(node)

    output_driver_path = afnode.parm('output_driver').eval()
    if output_driver_path:
        output_driver = hou.node(output_driver_path)
        if output_driver:
            nodes.insert(0, output_driver)
        else:
            hou.ui.displayMessage('Can`t find output drive node: "%s"' % output_driver_path)

    if afnode.parm('cmd_mode').eval():
        nodes.append(None)

    nodes.reverse()
    dependmask = ''
    prevparams = []
    for node in nodes:
        if node and node.isBypassed():
            continue

        newparams = []

        if node and node.type().name() == 'afanasy':
            newprefix = node.name()

            if prefix != '':
                newprefix = prefix + '_' + newprefix

            newparams = getJobParameters(node, True, frame_range, newprefix)
            dependmask = newprefix + '_.*'

            if newparams is None:
                return None

        elif node and node.type().name() == "wedge":
            wedgednode = None
            if node.inputs():
                wedgednode = node.inputs()[0]
            else:
                wedgednode = node.node(node.parm("driver").eval())
            if wedgednode == None:
                return None

            numWedges = computeWedge( node, node.type().name()) # we can remove nodetype check
            names = node.hdaModule().getwedgenames(node)
            for wedge in range(numWedges):
                # switching wedges like houdini do to get valid filenames
                hou.hscript('set WEDGE = ' + names[wedge])
                hou.hscript('set WEDGENUM = ' + str(wedge))
                hou.hscript('varchange')
                #add wedged node to next block
                block = getBlockParameters(afnode, wedgednode, subblock, "{}_{}".format(node.name(),wedge), frame_range)[0]
                block.auxargs += " --wedge " + node.path() + " --wedgenum " + str(wedge)
                newparams.append(block)
            # clear environment
            hou.hscript('set WEDGE = ')
            hou.hscript('set WEDGENUM = ')
            hou.hscript('varchange')

        else:
            newparams = \
                getBlockParameters(afnode, node, subblock, prefix, frame_range)

            if newparams is None:
                return None

            dependmask = newparams[0].name
            for param in newparams:
                if not param.valid:
                    return None

        if len(newparams):
            params.extend(newparams)
        else:
            return None

        if not afnode.parm('independent').eval() and dependmask != '':
            for prevparam in prevparams:
                prevparam.addDependMask(dependmask)

        prevparams = newparams

    # Last parameter needed to generate a job.
    if not subblock:
        params.append(
            BlockParameters(afnode, None, False, '', frame_range, True)
        )

    return params


def render(afnode):
    if VERBOSE:
        print('\nRendering "%s":' % afnode.path())

    params = getJobParameters(afnode)

    if params is not None and len(params) > 1:
        params[-1].genJob(params[:-1])
        for parm in params:
            parm.doPost()
    else:
        hou.ui.displayMessage(
            'No tasks found for:'
            '\n%s'
            '\nIs it connected to some valid ROP node?'
            '\nOr valid Output Driver should be specified.'
            % afnode.path()
        )


def computeWedge(ropnode, roptype):
    # check if this is a wedge rendering and compute the number of wedges
    numWedgeJobs = None
    if roptype == "wedge":
        if ropnode.parm("wedgemethod").eval() == 0:
            if ropnode.parm("random").eval() == 1:
                numWedgeJobs = ropnode.parm("numrandom").eval()
            else:
                numWedgeJobs = 1
                for parm in ropnode.parm("wedgeparams").multiParmInstances():
                    if parm.name().startswith("steps"):
                        if parm.eval() != 0:
                            numWedgeJobs *= parm.eval()
        else:
            parent = ropnode.parm("roottake").eval()
            parentTakes = hou.hscript("takels -iqp %s" % parent)[0].split("\n")
            takes = [takename for takename in parentTakes if takename]
            numWedgeJobs = len(takes)

        if not numWedgeJobs:
            raise hou.OperationFailed("The specified wedge node does not compute anyting.")
    return numWedgeJobs
