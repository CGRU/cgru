# -*- coding: utf-8 -*-
import nuke

import os
import re
import time

import cgruutils

RenderNodeClassName = 'Write'
DailiesNodeClassName = 'cgru_dailies'
AfanasyNodeClassName = 'afanasy'
AfanasyServiceType = 'nuke'
DailiesServiceType = 'movgen'

VERBOSE = 0

af = None


def checkFrameRange(framefirst, framelast, frameinc, framespertask, string=''):
    if string != '':
        string = ' on "%s"' % string
    if framefirst > framelast:
        nuke.message('First frame > Last frame' + string)
        return False
    if frameinc < 1:
        nuke.message('Frame increment must be >= 1' + string)
        return False
    if framespertask < 1:
        nuke.message('Frames per task must be >= 1' + string)
        return False

    tasksnum = \
        (1.0 + framelast - framefirst) / (1.0 * frameinc * framespertask)

    if tasksnum > 10000.0:
        if not nuke.ask('Tasks number over 10 000%s\nAre you sure?' % string):
            return False

    if tasksnum > 100000.0:
        if not nuke.ask('Tasks number over 100 000%s\nAre you sure?' % string):
            return False

    if tasksnum > 1000000.0:
        nuke.message(
            'Tasks number over 1 000 000%s\nPlease contact your supervisor, '
            'administrator or TD.' % string
        )
        return False

    return True


def getInputNodes(afnode, parent):
    if parent is None:
        print('Node is "None"')
        return None

    if VERBOSE == 2:
        print('Getting inputs of "%s"' % parent.name())

    global LastAfNode
    global InputNumber
    global InputName

    inputnodes = []
    for i in range(parent.inputs()):
        node = parent.input(i)
        if node is None:
            continue
        inputnodes.append(node)

    if afnode is not None:
        LastAfNode = afnode
        addnodes = afnode.knob('addnodes').value()
        if addnodes is not None and addnodes != '':
            if VERBOSE == 2:
                print('Adding nodes "%s" to "%s"' % (addnodes, afnode.name()))
            rexpr = re.compile(addnodes + '$')
            addnodes = []
            addnodes.extend(nuke.allNodes(RenderNodeClassName, nuke.root()))
            addnodes.extend(nuke.allNodes(AfanasyNodeClassName, nuke.root()))
            if len(addnodes) > 1:
                addnodes = sorted(addnodes, key = lambda node: node.name())
            for node in addnodes:
                if rexpr.match(node.name()):
                    inputnodes.append(node)

    nodes = []
    for i in range(len(inputnodes)):
        node = inputnodes[i]
        if afnode is not None:
            InputNumber = i + 1
            InputName = node.name()
        if node.Class() == RenderNodeClassName \
                or node.Class() == AfanasyNodeClassName \
                or node.Class() == DailiesNodeClassName:
            disableknob = node.knob('disable')
            if disableknob:
                if not disableknob.value():
                    nodes.append(node)
                else:
                    if VERBOSE == 1:
                        print('Node "%s" is disabled' % node.name())
                    continue
        else:
            if node.inputs() > 0:
                childs = getInputNodes(None, node)
                if childs is None:
                    return None
                if len(childs) > 0:
                    nodes.extend(childs)
            else:
                nuke.message(
                    'Leaf node reached "%s"-"%s" on "%s" input #%d.' %
                    (node.name(), InputName, LastAfNode.name(), InputNumber)
                )
                return None
    return nodes


class BlockParameters:
    def __init__(self, afnode, wnode, subblock, prefix, fparams):
        if VERBOSE == 2:
            print('Initializing block parameters for "%s"' % wnode.name())
        self.wnode = wnode
        self.valid = True

        self.subblock = subblock
        self.prefix = prefix

        self.framefirst = nuke.root().firstFrame()
        self.framelast = nuke.root().lastFrame()
        self.frameinc = 1
        self.framespertask = 1
        self.framesequential = 1
        self.skipexisting = 0
        self.maxruntasks = -1
        self.capacity = -1
        self.maxperhost = -1
        self.maxruntime = -1
        self.need_memory = -1
        self.need_gpu_mem = -1
        self.hostsmask = None
        self.hostsmaskexclude = None
        self.fullrangedepend = 0
        self.tmpimage = 1
        self.pathsmap = 1
        self.imgfiles = []
        self.moviemode = False
        self.tickets_use = 0
        self.tickets_data = None

        # Just to add to the final job name some info, for example timecode
        self.jobname_suffix = ''

        if afnode is not None:
            self.framefirst = int(afnode.knob('framefirst').value())
            self.framelast = int(afnode.knob('framelast').value())
            self.frameinc = int(afnode.knob('frameinc').value())
            self.framespertask = int(afnode.knob('framespertask').value())
            self.framesequential = int(afnode.knob('framesequential').value())
            self.skipexisting = int(afnode.knob('skipexisting').value())
            self.maxruntasks = int(afnode.knob('maxruntasks').value())
            self.capacity = int(afnode.knob('capacity').value())
            self.maxperhost = int(afnode.knob('maxperhost').value())
            self.maxruntime = int(afnode.knob('maxruntime').value())
            self.need_memory = int(afnode.knob('need_memory').value())
            self.need_gpu_mem = float(afnode.knob('need_gpu_mem').value())
            self.tmpimage = int(afnode.knob('tmpimage').value())
            self.pathsmap = int(afnode.knob('pathsmap').value())
            self.hostsmask = afnode.knob('hostsmask').value()
            self.hostsmaskexclude = afnode.knob('hostsmaskexcl').value()
            self.tickets_use = int(afnode.knob('tickets_use').value())
            self.tickets_data = afnode.knob('tickets_data').value()

            if int(afnode.knob('timecode_use').value()):
                timecode = afnode.knob('timecode').value()
                if len (timecode):
                    frange = cgruutils.timecodesToFrameRange( timecode, nuke.root().fps())
                    if frange is None or frange[0] is None or frange[1] is None:
                        nuke.message('Invalid timecode: "%s"' % timecode)
                        return None
                    self.framefirst = frange[0]
                    self.framelast  = frange[1]
                    afnode.knob('timecode').setValue( cgruutils.timecodesFromFrameRange( frange[0], frange[1]))

                    # Add timecode to a final job name:
                    self.jobname_suffix += '.' + timecode.replace(' ','')

        if self.skipexisting: self.framespertask = 1

        self.writename = str(wnode.fullName())

        if wnode.Class() == RenderNodeClassName:
            afcommon = __import__('afcommon', globals(), locals(), [])
            # Get images files:
            if nuke.toNode('root').knob('proxy').value():
                fileknob = wnode.knob('proxy')
            else:
                fileknob = wnode.knob('file')

            # Get views:
            views = wnode.knob('views')
            if views is not None:
                views = views.value()
                if views is None or views == '':
                    views = nuke.views()
                else:
                    views = views.split(' ')
            else:
                views = nuke.views()

            # Iterate views:
            for view in views:
                view = view.strip()
                if not len(view):
                    continue  # skip empty view, may be after split(' ')

                # Check view exists:
                if not view in nuke.views():
                    print('Error: Skipping invalid view: "%s"' % view)
                    continue

                    # if len( self.imgfiles):
                # self.imgfiles += ';'

                # Get show files for current view and first and last frames:
                octx = nuke.OutputContext()
                octx.setView(1 + nuke.views().index(view))
                octx_framefirst = self.framefirst
                octx_framelast = self.framelast
                if octx_framefirst < 0:
                    octx_framefirst = 0
                if octx_framelast < 0:
                    octx_framelast = 0

                # If frame first and frame last are equal no sequence needed
                if octx_framefirst == octx_framelast:
                    octx.setFrame(octx_framefirst)
                    self.imgfiles.append(fileknob.getEvaluatedValue(octx))
                else:
                    # Get files from first and last frames
                    # to calculate frames pattern:
                    octx.setFrame(octx_framefirst)
                    images1 = fileknob.getEvaluatedValue(octx)
                    if images1 is None or images1 == '':
                        nuke.message(
                            'Error:\n'
                            '%s\n'
                            'Files are empty.\n'
                            'View "%s", frame %d.' %
                            (self.writename, view, self.framefirst)
                        )
                        self.valid = False
                        return

                    if cgruutils.isMovieExt(images1):
                        self.imgfiles.append(images1)
                        self.moviemode = True
                        continue

                    octx.setFrame(octx_framelast)
                    images2 = fileknob.getEvaluatedValue(octx)
                    if images2 is None or images2 == '':
                        nuke.message(
                            'Error:\n'
                            '%s\n'
                            'Files are empty.\n'
                            'View "%s", frame %d.' %
                            (self.writename, view, self.framelast)
                        )
                        self.valid = False
                        return

                    part1, padding, part2 = afcommon.splitPathsDifference(images1, images2)

                    if padding < 1:
                        nuke.message(
                            'Error:\n'
                            '%s\n'
                            'Invalid files pattern.\n'
                            'View "%s".' %
                            (self.writename, view)
                        )
                        self.valid = False
                        return

                    self.imgfiles.append('%s@%s@%s' % (part1, '#' * padding, part2))

            # Check images folders:
            for imgfile in self.imgfiles:
                folder = os.path.dirname(imgfile)
                if folder != '':
                    if not os.path.isdir(folder):
                        result = nuke.ask(
                            'Write Node "%s" Directory\n'
                            '%s\n'
                            'Does not exist.\n'
                            'Create it?' % (self.writename, folder)
                        )
                        if result:
                            os.makedirs(folder)
                            if not os.path.isdir(folder):
                                nuke.message(
                                    "Can't create folder:\n%s" % folder
                                )
                                self.valid = False
                        else:
                            self.valid = False
        elif wnode.Class() == DailiesNodeClassName:
            if VERBOSE:
                print('Generating dailies "%s"' % self.writename)
        else:
            nuke.message('Node type\n"%s"\nis unsendable.' % self.writename)
            self.valid = False

        for par in fparams:
            if fparams[par] is not None:
                if hasattr(self, par):
                    setattr(self, par, fparams[par])

        if self.moviemode:
            self.framespertask = self.framelast - self.framefirst + 1

        self.name = self.writename
        if subblock:
            if self.prefix is not None:
                if self.prefix != '':
                    self.name = self.prefix + self.name

        self.dependmask = ''
        self.tasksdependmask = ''

    def cinergy_block_environs(self,block):
        cinergy=None
        assetids=''
        try:
            import pipeline
        except:
            pass
        try:
            import cinergy
        except:
            pass
        if cinergy !=None:
            asset_ids = cinergy.scan_asset_ids(nuke.allNodes())
            for a in asset_ids:
                assetids+=str(a)+'|'
            if assetids.endswith('|'):
                assetids=assetids[:-1]
            block.setEnv('CNM_ASSETS_IDS',assetids)

        if pipeline is not None:
            context_file=nuke.root().name()
            block.setEnv('CNM_CONTEXT_FILE',context_file)
            context = pipeline.get_context(filename=context_file)
            for k in context.keys():
                block.setEnv('CNM_'+k.upper(),str(context[k]))
        return block


    def addTasksDependMask(self, mask):
        if self.tasksdependmask == '':
            self.tasksdependmask = mask
        else:
            self.tasksdependmask = self.tasksdependmask + '|' + mask

    def addDependMask(self, mask):
        if self.dependmask == '':
            self.dependmask = mask
        else:
            self.dependmask = self.dependmask + '|' + mask

    def genBlock(self, i_scene_path):
        if VERBOSE == 2:
            print('Generating block "%s"' % self.name)

        if not self.valid:
            return

        if self.wnode.Class() == RenderNodeClassName:
            block = af.Block(self.name, AfanasyServiceType)
            block.setNumeric(self.framefirst, self.framelast,
                             self.framespertask, self.frameinc)
            print(self.imgfiles)
            block.setFiles(self.imgfiles)
            if self.skipexisting:
                block.skipExistingFiles()
            if self.framesequential != 1:
                block.setSequential( self.framesequential)
            if self.capacity != -1:
                block.setCapacity(self.capacity)
            if self.maxruntime != -1:
                block.setTasksMaxRunTime(60 * self.maxruntime)
            if self.need_memory > 0:
                block.setNeedMemory(1024 * self.need_memory)
            if self.need_gpu_mem > 0:
                block.setNeedGPUMemGB(self.need_gpu_mem)
            if self.tickets_use and self.tickets_data is not None and len(self.tickets_data):
                for ticket in self.tickets_data.split(','):
                    ticket = ticket.strip().split(':')
                    if len(ticket) != 2:
                        nuke.message('Invalid ticket data: "%s".' % ticket)
                        continue
                    block.addTicket(ticket[0], int(ticket[1]))

            cmd = os.getenv('NUKE_AF_RENDER', 'nuke')

            cmd += ' --version={0}'.format(nuke.NUKE_VERSION_STRING)

            if self.tmpimage or self.pathsmap:
                cgru_location = os.getenv('CGRU_LOCATION')
                if cgru_location is None:
                    print(
                        'CGRU_LOCATION is not set, can`t enable tmpimage '
                        'and pathsmap'
                    )
                else:
                    # That the Nuke-Render-Script need to
                    # rewrite the Script and replace file to render in
                    # temporary image file
                    nukerenderscript = os.environ['CGRU_LOCATION']
                    nukerenderscript = os.path.join(nukerenderscript,
                                                    'plugins')
                    nukerenderscript = os.path.join(nukerenderscript, 'nuke')
                    nukerenderscript = os.path.join(nukerenderscript,
                                                    'render.py')
                    cmd += ' -t %s' % nukerenderscript
                    if not self.tmpimage:
                        cmd += ' --notmpimage'
                    if not self.pathsmap:
                        cmd += ' --nopathsmap'

            cmd += ' -X %s -F@#@-@#@x%d -x \"%s\"' % \
                   (self.writename, self.frameinc, i_scene_path)

            block.setCommand(cmd)

        elif self.wnode.Class() == DailiesNodeClassName:
            cgru = __import__('cgru', globals(), locals(), [])
            cgru.dailiesEvaluate(self.wnode)
            cmd = cgru.dailiesGenCmd(self.wnode)
            if cmd is None or cmd == '':
                return
            block = af.Block(
                os.path.basename(cmd.split(' ')[-1]),
                DailiesServiceType
            )

            if self.capacity != -1:
                block.setCapacity(self.capacity)

            task = af.Task('dailies')
            task.setCommand(cmd)
            block.tasks.append(task)
        else:
            print('Invalid block node class = "%s"' % self.wnode.Class())
            return

        if self.dependmask != '':
            block.setDependMask(self.dependmask)

        if self.tasksdependmask != '':
            block.setTasksDependMask(self.tasksdependmask)

        if self.subblock:
            if self.maxruntasks != -1:
                block.setMaxRunningTasks(self.maxruntasks)
            if self.maxperhost != -1:
                block.setMaxRunTasksPerHost(self.maxperhost)
            if self.hostsmask is not None:
                self.hostsmask = str(self.hostsmask)
                if self.hostsmask != '':
                    block.setHostsMask(self.hostsmask)
            if self.hostsmaskexclude is not None:
                self.hostsmaskexclude = str(self.hostsmaskexclude)
                if self.hostsmaskexclude != '':
                    block.setHostsMaskExclude(self.hostsmaskexclude)
        self.cinergy_block_environs(block=block)

        return block


def getBlocksParameters(afnode, subblock, prefix, fparams):
    if VERBOSE == 2:
        print('Getting block parameters "%s"' % afnode.name())

    # Get parameters:
    framefirst = int(afnode.knob('framefirst').value())
    framelast = int(afnode.knob('framelast').value())
    frameinc = int(afnode.knob('frameinc').value())
    framespertask = int(afnode.knob('framespertask').value())
    framesequential = int(afnode.knob('framesequential').value())
    independent = int(afnode.knob('independent').value())
    reversedepends = int(afnode.knob('reversedeps').value())
    forceframes = int(afnode.knob('forceframes').value())
    if not checkFrameRange(framefirst, framelast, frameinc, framespertask,
                           afnode.name()):
        return

    if forceframes:
        fparams = {
            'framefirst': framefirst,
            'framelast': framelast,
            'frameinc': frameinc,
            'framespertask': framespertask,
            'framesequential': framesequential
        }

    # MutiWrite parameters:
    if subblock:
        newprefix = afnode.name()
        jobname = afnode.knob('jobname').value()
        if jobname is not None and jobname != '':
            newprefix = jobname
        newprefix += '-'
        if prefix is None or prefix == '':
            prefix = newprefix
        else:
            prefix += newprefix

    blocksparams = []

    # Get input render nodes:
    nodes = getInputNodes(afnode, afnode)
    if nodes is None:
        return None
    if len(nodes) == 0:
        return blocksparams

    prevparams = []
    # Process input nodes:
    for node in nodes:
        newparams = []
        if node.Class() == AfanasyNodeClassName:
            # Recursion if input node class is "afanasy" too
            newparams = getBlocksParameters(node, True, prefix, fparams)
            # Get fullrangedepend only if afanasy connected to afanasy node
            for newparam in newparams:
                newparam.fullrangedepend = int(node.knob('fullrange').value())
            if newparams is None:
                return
            if len(newparams) == 0:
                continue
        else:
            # Get block parameters from node:
            bparams = BlockParameters(afnode, node, subblock, prefix, fparams)
            if not bparams.valid:
                return None
            newparams.append(bparams)

        # Set dependencies:
        if not independent:
            if len(prevparams) > 0:
                if reversedepends:
                    if len(newparams) > 1:
                        mask = newparams[0].prefix + '.*'
                    else:
                        mask = newparams[0].name
                    for ppar in prevparams:
                        if newparams[0].fullrangedepend:
                            ppar.addDependMask(mask)
                        else:
                            ppar.addTasksDependMask(mask)
                else:
                    if len(prevparams) > 1:
                        mask = prevparams[0].prefix + '.*'
                    else:
                        mask = prevparams[0].name
                    for nparam in newparams:
                        if prevparams[0].fullrangedepend:
                            nparam.addDependMask(mask)
                        else:
                            nparam.addTasksDependMask(mask)

        # Store previous parameters for dependencies:
        prevparams = newparams

        # Append parameters array:
        blocksparams.extend(newparams)

    return blocksparams


class JobParameters:
    def __init__(self, afnode, jobname, blocks, fparams):
        self.valid = True
        if VERBOSE == 2:
            nodename = '???'
            if afnode is not None:
                nodename = afnode.name()
            elif len(blocks):
                nodename = blocks[0].name
            print('Initializing job parameters: "%s"' % nodename)

        self.startpaused = 0
        self.maxruntasks = -1
        self.maxperhost = -1
        self.priority = -1
        self.platform = None
        self.hostsmask = None
        self.hostsmaskexclude = None
        self.dependmask = None
        self.dependmaskglobal = None
        self.nodename = None
        self.tmpscene = 1
        self.tmpimage = 1
        self.pathsmap = 1
        self.pools_use = 0
        self.pools_data = None

        if afnode is not None:
            self.startpaused = int(afnode.knob('startpaused').value())
            self.maxruntasks = int(afnode.knob('maxruntasks').value())
            self.maxperhost = int(afnode.knob('maxperhost').value())
            self.priority = int(afnode.knob('priority').value())
            self.platform = afnode.knob('platform').value()
            self.hostsmask = afnode.knob('hostsmask').value()
            self.hostsmaskexclude = afnode.knob('hostsmaskexcl').value()
            self.dependmask = afnode.knob('dependmask').value()
            self.dependmaskglobal = afnode.knob('dependmaskglbl').value()
            self.nodename = afnode.name()
            self.tmpscene = int(afnode.knob('tmpscene').value())
            self.tmpimage = int(afnode.knob('tmpimage').value())
            self.pathsmap = int(afnode.knob('pathsmap').value())
            self.pools_use = int(afnode.knob('pools_use').value())
            self.pools_data = afnode.knob('pools_data').value()

        self.blocksparameters = []

        self.jobname = jobname
        self.prefix = self.jobname

        for par in fparams:
            if fparams[par] is not None:
                if hasattr(self, par):
                    setattr(self, par, fparams[par])

        if blocks is None:
            self.blocksparameters = \
                getBlocksParameters(afnode, False, '', fparams)
            if self.blocksparameters is None:
                self.valid = False
        else:
            self.blocksparameters.extend(blocks)
            blocksnames = blocks[0].name
            if len(blocks) > 1:
                for i in range(1, len(blocks)):
                    blocksnames += '-' + blocks[i].name
            self.jobname += '-' + blocksnames
            self.prefix += '-'
            if self.nodename is None:
                self.nodename = blocksnames

    def addDependMask(self, mask):
        if self.dependmask is None or self.dependmask == '':
            self.dependmask = mask
        else:
            self.dependmask = self.dependmask + '|' + mask

    def genJob(self, i_scene_path):
        if VERBOSE == 2:
            print('Generating job on: "%s"' % self.nodename)

        if not self.valid:
            return None

        if self.blocksparameters is None:
            if VERBOSE:
                print(
                    'Block parameters generation error on "%s"' % self.nodename
                )
            return
        if len(self.blocksparameters) == 0:
            if VERBOSE:
                print('No blocks parameters generated on "%s"' % self.nodename)
            return

        afcommon = __import__('afcommon', globals(), locals(), [])
        if self.tmpscene:
            # Add job name to temporary scene file
            self.scene_path = i_scene_path + afcommon.filterFileName('.%s.nk' % self.jobname)
        else:
            self.scene_path = i_scene_path

        jobname = str(self.jobname)

        blocks = []
        for bparams in self.blocksparameters:
            block = bparams.genBlock(self.scene_path)
            if block is None:
                if VERBOSE:
                    print('Block generation error on "%s" - "%s"' %
                          (self.nodename, bparams.name))
                return
            blocks.append(block)
            jobname += bparams.jobname_suffix

        if len(blocks) == 0:
            if VERBOSE:
                print('No blocks generated error on "%s"' % self.nodename)
            return

        job = af.Job( jobname)
        if self.priority != -1:
            job.setPriority(self.priority)
        if self.maxruntasks != -1:
            job.setMaxRunningTasks(self.maxruntasks)
        if self.maxperhost != -1:
            job.setMaxRunTasksPerHost(self.maxperhost)
        if self.hostsmask is not None:
            self.hostsmask = str(self.hostsmask)
            if self.hostsmask != '':
                job.setHostsMask(self.hostsmask)
        if self.hostsmaskexclude is not None:
            self.hostsmaskexclude = str(self.hostsmaskexclude)
            if self.hostsmaskexclude != '':
                job.setHostsMaskExclude(self.hostsmaskexclude)
        if self.dependmask is not None:
            self.dependmask = str(self.dependmask)
            if self.dependmask != '':
                job.setDependMask(self.dependmask)
        if self.dependmaskglobal is not None:
            self.dependmaskglobal = str(self.dependmaskglobal)
            if self.dependmaskglobal != '':
                job.setDependMaskGlobal(self.dependmaskglobal)

        if self.pools_use and self.pools_data is not None and len(self.pools_data):
            pools = dict()
            for pool in self.pools_data.split(','):
                pool = pool.split(':')
                if len(pool) != 2:
                    continue
                pools[pool[0]] = int(pool[1])
            job.setPools(pools)

        if self.startpaused:
            job.offline()
        if self.platform is None or self.platform == 'Any':
            job.setAnyOS()
        else:
            job.setNativeOS()
        job.setCmdPost('deletefiles "%s"' % self.scene_path)

        job.blocks = blocks

        # Folders (for GUI only):
        f_output = None
        for block in blocks:
            if 'files' in block.data and len(block.data['files']):
                f_output = block.data['files'][0]
                break
        if f_output is not None:
            job.setFolder('output', os.path.dirname( f_output))
        job.setFolder('input', os.path.dirname( nuke.root().name()))

        return job


def getJobsParameters(afnode, prefix, fparams):
    if VERBOSE == 2:
        print('Generating jobs parameters on "%s"' % afnode.name())
    jobsparameters = []
    if afnode.knob('disable') is True:
        if VERBOSE == 1:
            print('Node "%s" is disabled' % afnode.name())
        return jobsparameters

    # Get parameters:
    framefirst = int(afnode.knob('framefirst').value())
    framelast = int(afnode.knob('framelast').value())
    frameinc = int(afnode.knob('frameinc').value())
    framespertask = int(afnode.knob('framespertask').value())
    framesequential = int(afnode.knob('framesequential').value())
    independent = int(afnode.knob('independent').value())
    reversedepends = int(afnode.knob('reversedeps').value())
    forceframes = int(afnode.knob('forceframes').value())
    singlejob = int(afnode.knob('singlejob').value())

    jobname = afnode.knob('jobname').value()
    if jobname is None or jobname == '':
        jobname = afnode.name()
    jobname = prefix + '-' + jobname

    if not checkFrameRange(framefirst, framelast, frameinc, framespertask,
                           afnode.name()):
        return

    if forceframes:
        fparams = {
            'framefirst': framefirst,
            'framelast': framelast,
            'framespertask': framespertask,
            'framesequential': framesequential
        }

    # Construct single job (no jobs recursion)
    if singlejob:
        jobparams = JobParameters(afnode, jobname, None, fparams)
        if not jobparams.valid:
            return None
        jobsparameters.append(jobparams)
        return jobsparameters

    # Construct a job for each connection
    nodes = getInputNodes(afnode, afnode)
    if nodes is None:
        return None
    if len(nodes) == 0:
        return jobsparameters

    # Reverse nodes, to send most depended job last
    if reversedepends:
        nodes.reverse()

    # Construct job parameters
    dependname = None
    for node in nodes:
        if node.Class() == AfanasyNodeClassName:
            # Recursion if "afanasy" node connected
            newjobparams = getJobsParameters(node, jobname, fparams)
            if newjobparams is None:
                return
            if len(newjobparams) == 0:
                continue
            # Set dependencies
            if not independent:
                if dependname is not None:
                    for jobparams in newjobparams:
                        jobparams.addDependMask(dependname + '.*')
                dependname = newjobparams[0].prefix
            # Extend parameters array
            jobsparameters.extend(newjobparams)
        else:
            # Generate a job with one block if "write" node connected
            blocksparameters = []
            bparams = BlockParameters(afnode, node, False, '', fparams)
            if not bparams.valid:
                return None
            blocksparameters.append(bparams)
            jobparams = JobParameters(afnode, prefix, blocksparameters,
                                      fparams)
            if not jobparams.valid:
                return None
            # Set dependencies
            if not independent:
                if dependname is not None:
                    jobparams.addDependMask(dependname + '.*')
                dependname = jobparams.jobname
            # Append parameters array
            jobsparameters.append(jobparams)

    return jobsparameters


def renderNodes(nodes, fparams, storeframes):
    global af
    af = __import__('af', globals(), locals(), [])

    scene_path = nuke.root().name()
    if scene_path == 'Root':
        scene_path = os.getenv('NUKE_AF_TMPSCENE', 'tmp')
    scene_path = os.path.abspath(scene_path)
    scene_name = os.path.basename(scene_path)
    ftime = time.time()
    tmp_suffix = time.strftime('.%m%d-%H%M%S-') + str(ftime - int(ftime))[2:5]
    tmp_scene_path = scene_path + tmp_suffix

    jobsparameters = []
    for node in nodes:
        newjobparameters = []
        newjobparameters = None
        if node.Class() == AfanasyNodeClassName:
            oldparams = dict()
            for key in fparams:
                oldparams[key] = node.knob(key).value()
                node.knob(key).setValue(fparams[key])
            newjobparameters = getJobsParameters(node, scene_name, dict())
            if newjobparameters is None:
                return
            if not storeframes:
                for key in oldparams:
                    node.knob(key).setValue(oldparams[key])
        if node.Class() == RenderNodeClassName \
                or node.Class() == DailiesNodeClassName:
            blocksparameters = []
            bparams = BlockParameters(None, node, False, '', fparams)
            if not bparams.valid:
                return
            blocksparameters.append(bparams)
            jobparams = JobParameters(None, scene_name, blocksparameters,
                                      fparams)
            if not jobparams.valid:
                return

            newjobparameters = [jobparams]

        if newjobparameters is None:
            if VERBOSE:
                print('Job(s) parameters generation error on "%s"' %
                      node.name())
            return
        if len(newjobparameters) > 0:
            jobsparameters.extend(newjobparameters)

    jobs = []
    for jobparams in jobsparameters:
        if jobparams.tmpscene:
            job = jobparams.genJob(tmp_scene_path)
        else:
            job = jobparams.genJob(scene_path)
        if job is None:
            if VERBOSE:
                print('Job generatiton error on "%s"' % jobparams.nodename)
            return
        jobs.append(job)

    if len(jobs) == 0:
        nuke.message('No jobs generated.')
        return

    cgrupathmap = __import__('cgrupathmap', globals(), locals(), [])
    pm = cgrupathmap.PathMap(UnixSeparators=True, Verbose=False)

    # Store scene modified state
    changed = nuke.modified()

    # Save all generated jobs scenes
    for i in range(len(jobs)):
        scene_path = jobsparameters[i].scene_path

        # Apply paths mapping if enables and rendering to temporary scene
        if jobsparameters[i].pathsmap and pm.initialized and jobsparameters[i].tmpscene:
            pm_scene_path = scene_path + '.pm'
            nuke.scriptSave(pm_scene_path)
            pm.toServerFile(
                pm_scene_path,
                scene_path,
                SearchStrings=['file ', 'font ', 'project_directory '],
                Verbose=False
            )
            os.remove(pm_scene_path)
        else:
            nuke.scriptSave(scene_path)

        # Send job to server
        if not jobs[i].send()[0]:
            nuke.message('Unable to send job to server.')
            os.remove(scene_path)
            break

        # This is needed if ocassionaly user sending to render thousands of nodes
        time.sleep(0.1)

    # Restore scene modified state
    nuke.modified(changed)


def render(node=None):
    nodes = []
    fparams = dict()

    if node is not None:
        # Render only specified node:
        nodes.append(node)
        renderNodes(nodes, fparams, False)
        return

    # Store minimum and maximum frames to show in dialog
    hasafanasynodes = False
    framefirst_min = None
    framefirst_max = None
    framelast_min = None
    framelast_max = None
    framespertask_min = None
    framespertask_max = None
    framesequential_min = None
    framesequential_max = None
    selectednodes = sorted(nuke.selectedNodes(), key = lambda node: node.name())
    for node in selectednodes:
        if node.Class() == AfanasyNodeClassName \
                or node.Class() == RenderNodeClassName \
                or node.Class() == DailiesNodeClassName:
            nodes.append(node)
            # Check for minimum and maximum
            if node.Class() == AfanasyNodeClassName:
                hasafanasynodes = True
                framefirst = int(node.knob('framefirst').value())
                framelast = int(node.knob('framelast').value())
                framespertask = int(node.knob('framespertask').value())
                framesequential = int(node.knob('framesequential').value())
            else:
                framefirst = nuke.root().firstFrame()
                framelast = nuke.root().lastFrame()
                framespertask = 1 + int((framelast - framefirst) / 10.0)
                framesequential = 1

            if framefirst_min is None:
                framefirst_min = framefirst
            else:
                framefirst_min = min(framefirst_min, framefirst)
            if framefirst_max is None:
                framefirst_max = framefirst
            else:
                framefirst_max = max(framefirst_max, framefirst)

            if framelast_min is None:
                framelast_min = framelast
            else:
                framelast_min = min(framelast_min, framelast)
            if framelast_max is None:
                framelast_max = framelast
            else:
                framelast_max = max(framelast_max, framelast)

            if framespertask_min is None:
                framespertask_min = framespertask
            else:
                framespertask_min = min(framespertask_min, framespertask)
            if framespertask_max is None:
                framespertask_max = framespertask
            else:
                framespertask_max = max(framespertask_max, framespertask)

            if framesequential_min is None:
                framesequential_min = framesequential
            else:
                framesequential_min = min(framesequential_min, framesequential)
            if framesequential_max is None:
                framesequential_max = framesequential
            else:
                framesequential_max = max(framesequential_max, framesequential)

    if len(nodes) < 1:
        nuke.message(
            'No nodes to render found.\n'
            'Select "%s" or "%s" node(s) to render.' %
            (AfanasyNodeClassName, RenderNodeClassName)
        )
        return

    nodesstring = nodes[0].name()
    if len(nodes) > 1:
        for i in range(1, len(nodes)):
            nodesstring += ' ' + nodes[i].name()

    # Construct frame ranges:
    if framefirst_min != framefirst_max:
        framefirst = '%s..%s' % (framefirst_min, framefirst_max)
    else:
        framefirst = framefirst_min

    if framelast_min != framelast_max:
        framelast = '%s..%s' % (framelast_min, framelast_max)
    else:
        framelast = framelast_min

    if framespertask_min != framespertask_max:
        framespertask = '%s..%s' % (framespertask_min, framespertask_max)
    else:
        framespertask = framespertask_min

    if framesequential_min != framesequential_max:
        framesequential = '%s..%s' % (framesequential_min, framesequential_max)
    else:
        framesequential = framesequential_min

    # Dialog:
    panel = nuke.Panel('Afanasy Render')
    panel.addSingleLineInput('Nodes:', nodesstring)
    panel.addSingleLineInput('First Frame:', framefirst)
    panel.addSingleLineInput('Last Frame:', framelast)
    panel.addSingleLineInput('Frames Per Task:', framespertask)
    panel.addSingleLineInput('Frame Sequential:', framesequential)
    if hasafanasynodes:
        panel.addBooleanCheckBox('Store Frames Settings', 0)
    panel.addBooleanCheckBox('Start Paused', 0)
    panel.addButton("Cancel")
    panel.addButton("OK")
    result = panel.show()
    if not result:
        return

    # Check for selected nodes:
    nodesstring = panel.value('Nodes:')
    selectednodes = nodesstring.split()
    nodes = []
    for name in selectednodes:
        node = nuke.toNode(name)
        if node is None:
            nuke.message('Node "%s" not found.' % name)
            return
        if node.Class() == AfanasyNodeClassName \
                or node.Class() == RenderNodeClassName \
                or node.Class() == DailiesNodeClassName:
            nodes.append(node)

    if len(nodes) < 1:
        nuke.message(
            'No nodes to render found.\n'
            'Select "%s" or "%s" node(s) to render.' %
            (AfanasyNodeClassName, RenderNodeClassName)
        )
        return

    # Get parameters:
    sframefirst = str(panel.value('First Frame:'))
    sframelast = str(panel.value('Last Frame:'))
    sframespertask = str(panel.value('Frames Per Task:'))
    sframesequential = str(panel.value('Frame Sequential:'))
    storeframes = False
    if hasafanasynodes:
        storeframes = int(panel.value('Store Frames Settings'))
    if panel.value('Start Paused'):
        fparams['startpaused'] = 1

    # Check frame range was set:
    if sframefirst.find('..') == -1:
        try:
            framefirst = int(sframefirst)
        except:
            nuke.message('Invalid first frame "%s"' % sframefirst)
            return
        fparams['framefirst'] = framefirst

    if sframelast.find('..') == -1:
        try:
            framelast = int(sframelast)
        except:
            nuke.message('Invalid last frame "%s"' % sframelast)
            return
        fparams['framelast'] = framelast

    if sframespertask.find('..') == -1:
        try:
            framespertask = int(sframespertask)
        except:
            nuke.message('Invalid frames per task "%s"' % sframespertask)
            return
        fparams['framespertask'] = framespertask

    if sframesequential.find('..') == -1:
        try:
            framesequential = int(sframesequential)
        except:
            nuke.message('Invalid frames sequential "%s"' % sframesequential)
            return
        fparams['framesequential'] = framesequential

    if not checkFrameRange(framefirst, framelast, 1, framespertask):
        return

    # Render selected nodes:
    renderNodes(nodes, fparams, storeframes)


def setFramesFromTimeCode( i_afnode):

    timecode = i_afnode.knob('timecode').value()

    if len(timecode) == 0:
        nuke.message('Timecode is empty.')
        return

    frange = cgruutils.timecodesToFrameRange( timecode, nuke.root().fps())
    if frange is None or frange[0] is None or frange[1] is None:
        nuke.message('Invalid timecode: "%s"' % timecode)
        return

    i_afnode.knob('framefirst').setValue( frange[0])
    i_afnode.knob('framelast').setValue( frange[1])
    i_afnode.knob('timecode').setValue( cgruutils.timecodesFromFrameRange( frange[0], frange[1]))

def setTimeCodeFromFrames( i_afnode):
    ffirst = int(i_afnode.knob('framefirst').value())
    flast = int(i_afnode.knob('framelast').value())
    i_afnode.knob('timecode').setValue( cgruutils.timecodesFromFrameRange( ffirst, flast))

