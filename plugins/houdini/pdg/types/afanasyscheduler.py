"""
To reload scheduler type:
import pdg; pdg.TypeRegistry.types().registeredType(pdg.registeredType.Scheduler, "afanasyscheduler").reload()
"""
import json
import os
import socket
import sys
import traceback

import hou
import pdg
from pdg.scheduler import PyScheduler
from pdg.job.callbackserver import CallbackServerMixin
from pdg.utils import expand_vars

import cgruconfig
import af


def displayError(msg, exception=None):
    """Pop up a message dialog to display the given error message.
    If the ui is unavailable, then it writes the message to the console.
    """
    if hou.isUIAvailable():
        details = (str(exception) if exception is not None else None)
        hou.ui.displayMessage(msg, severity=hou.severityType.Error, details=details)
    else:
        if exception is not None:
            msg += "\n" + str(exception)
        raise hou.OperationFailed(msg)

def displayMessage(msg):
    """Pop up a message dialog to display the given message.
    If the ui is unavailable, then it writes the message to the console.
    """
    if hou.isUIAvailable():
        hou.ui.displayMessage(msg, severity=hou.severityType.Message)
    else:
        print(msg)


class AfanasyScheduler(CallbackServerMixin, PyScheduler):
    """
    Scheduler implementation that interfaces with a Afanasy farm instance.
    """
    def __init__(self, scheduler, name):
        PyScheduler.__init__(self, scheduler, name)
        CallbackServerMixin.__init__(self, False)
        self.parmprefix = 'afanasy'
        self._initData()


    @classmethod
    def templateName(cls):
        return 'afanasyscheduler'


    @classmethod
    def templateBody(cls):
        return json.dumps({
            "name": "afanasyscheduler",
            "parameters" : []
        })


    def _initData(self):
        self.job_id = None
        self.job_block_name_id = {}
        self.job_tasks_id_workid = {}
        self._local_addr = self._getLocalAddr()


    def _log(self, i_msg):
        txt = str(i_msg)
        if self.topNode():
            txt = '%s: %s' % (self.topNode().name(), txt)
        print(txt)


    def _getTicketsDictFromString(self, i_str):
        tickets = dict()

        if i_str is None or len(i_str) == 0:
            return tickets

        for ticket in i_str.split(','):
            ticket = ticket.strip().split(':')
            if len(ticket) != 2:
                displayMessage('Invalid ticket data: "%s".' % ticket)
            tickets[ticket[0]] = int(ticket[1])

        return tickets


    def _getWorkItemServiceParserTickets(self, i_work_item):
        # Set the default service values
        service = 'hbatch'
        # PDG uses "ALF_PROGRESS" everywhere
        parser = 'mantra'
        # By default there are no tickets at all
        tickets = dict()
        tickets_auto = self['afanasy_tickets_auto'].evaluateInt()
        if tickets_auto:
            # If auto tickets, almost all nodes launches hython
            tickets['HYTHON'] = 1

        topnode = i_work_item.node.topNode()
        toptype = topnode.type().name()
        if toptype == 'ropfetch':
            # Try to detect ROP type
            roppath = topnode.parm('roppath')
            if roppath is not None:
                ropnode = hou.node(roppath.eval())
                if ropnode:
                    roptype = ropnode.type().name()
                    if roptype == 'ifd':
                        service = 'hbatch_mantra'
                        if tickets_auto:
                            tickets['MANTRA'] = 1
        elif toptype == 'ffmpegencodevideo':
            service = 'ffmpeg'
            parser = 'ffmpeg'
            tickets.pop('HYTHON', None)

        # Service can be specified directly:
        value = self.evaluateStringOverride(i_work_item.node, self.parmprefix, 'service', i_work_item, '')
        if value is not None and len(value):
            service = value

        # Parser can be specified directly:
        value = self.evaluateStringOverride(i_work_item.node, self.parmprefix, 'parser', i_work_item, '')
        if value is not None and len(value):
            parser = value

        # Add tickets that are specified directly:
        value = self.evaluateStringOverride(i_work_item.node, self.parmprefix, 'tickets', i_work_item, '')
        tickets.update(self._getTicketsDictFromString(value))

        return service, parser, tickets


    def _constructJob(self):
        job = af.Job(self['job_name'].evaluateString())
        job.setBranch(self['job_branch'].evaluateString())
        job.setPriority(self['priority'].evaluateInt())
        job.setMaxRunningTasks(self['afanasy_max_running_tasks'].evaluateInt())
        job.setMaxRunTasksPerHost(self['afanasy_max_running_tasks_per_host'].evaluateInt())
        job.setHostsMask(self['afanasy_hosts_mask'].evaluateString())
        job.setHostsMaskExclude(self['afanasy_hosts_mask_exclude'].evaluateString())

        return job


    def _constructBlock(self, work_item):
        service, parser, tickets = self._getWorkItemServiceParserTickets(work_item)
        block = af.Block(work_item.node.name, service)
        block.setParser(parser)
        for name in tickets:
            block.addTicket(name, tickets[name])
        block.setCapacity(self.evaluateIntOverride(work_item.node, self.parmprefix, 'capacity', work_item, -1))
        block.setHostsMask(self.evaluateStringOverride(work_item.node, self.parmprefix, 'hosts_mask', work_item, ''))
        block.setHostsMaskExclude(self.evaluateStringOverride(work_item.node, self.parmprefix, 'hosts_mask_exclude', work_item, ''))
        block.setMaxRunningTasks(self.evaluateIntOverride(work_item.node, self.parmprefix, 'max_running_tasks', work_item, -1))
        block.setMaxRunTasksPerHost(self.evaluateIntOverride(work_item.node, self.parmprefix, 'max_running_tasks_per_host', work_item, -1))
        block.setNeedMemory(self.evaluateIntOverride(work_item.node, self.parmprefix, 'need_memory', work_item, -1)*1024)
        block.setTaskMinRunTime(self.evaluateIntOverride(work_item.node, self.parmprefix, 'task_min_run_time', work_item, -1))
        block.setTaskMaxRunTime(int(self.evaluateFloatOverride(work_item.node, self.parmprefix, 'task_max_run_time', work_item, -1)*3600.0))
        env_dict, removekeys = self.resolveEnvParams(self.parmprefix, work_item, False)
        for name in env_dict:
            block.setEnv(name, env_dict[name])

        return block


    def _constructTask(self, work_item):
        task = af.Task(work_item.name)
        cmd = self.expandCommandTokens(work_item.command, work_item)

        if "REZ_USED_RESOLVE" in os.environ:
                cmd = 'rez-env {} --no-local -- {}'.format(
                    os.environ["REZ_USED_RESOLVE"],
                    cmd
                )

        task.setCommand(cmd)
        
        # Set environment variables
        task.setEnv('PDG_RESULT_SERVER', str(self.workItemResultServerAddr()))
        task.setEnv('PDG_ITEM_NAME', str(work_item.name))
        task.setEnv('PDG_DIR', str(self.workingDir(False)))
        task.setEnv('PDG_TEMP', str(self.tempDir(False)))
        task.setEnv('PDG_SHARED_TEMP', str(self.tempDir(False)))
        task.setEnv('PDG_INDEX', str(work_item.index))
        task.setEnv('PDG_INDEX4', "{:04d}".format(work_item.index))
        task.setEnv('PDG_SCRIPTDIR', str(self.scriptDir(False)))

        # This adds paths mapping data to environment
        self.resolvePathMapping(task.data['environment'])
        # We need to set it after task.setEnv,
        # as on task.setEnv CGRU paths mapping applied.
        # And better not to apply CGRU paths mapping on PDG paths mapping data.

        return task


    def _sendJob(self, job):
        try:
            self.job_id = job.send()
            self.job_id = self.job_id[1]['id']
        except:
            traceback.print_exc()
            sys.stderr.flush()
            self.job_id = None
            return False
        return True


    def _deleteJob(self):
        if self.job_id is not None:
            if self['keep_job_on_cancel_cook'].evaluateInt():
                af.Cmd().stopJobById(self.job_id)
            else:
                af.Cmd().deleteJobById(self.job_id)
        self._initData()


    def _getLocalAddr(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            # doesn't even have to be reachable
            s.connect(('10.255.255.255', 1))
            addr = s.getsockname()[0]
        except Exception:
            addr = socket.gethostbyname(socket.gethostname())
        finally:
            s.close()
        """ Solution from:
        https://stackoverflow.com/questions/166506/finding-local-ip-addresses-using-pythons-stdlib?page=1&tab=votes#tab-top
        """
        return addr

    def workItemResultServerAddr(self):
        # By default it uses local host name.
        addr = self._workItemResultServerAddr
        # On farm better to use direct IP address.
        if self['use_ip_address'].evaluateInt():
            addr, port = self._workItemResultServerAddr.split(':')
            addr = ':'.join([self._local_addr, port])
        return addr

    def applicationBin(self, i_app, i_work_item):
        """
        [virtual] Returns the path to the given application
        """
        # This is needed to locate such programs as hython, python.
        # CGRU has an own mechanism to locate soft,
        # and has such commands as hython, python in PATH.
        # So we can just return app name, no full app path is needed.
        return i_app


    def onStart(self):
        """
        [virtual] Called by PDG when scheduler is first created.
        """
        self._log('onStart')
        return True


    def onStop(self):
        """
        [virtual] Called by PDG when scheduler is cleaned up.
        """
        self._log('onStop')
        self.stopCallbackServer()
        self._deleteJob()
        return True


    def onStartCook(self, static, cook_set):
        """
        onStartCook(self, static, cook_set) -> boolean

        [virtual] Cook start callback. Starts the job for the cook session.
        """
        self._log('onStartCook')
        self._deleteJob()

        pdg_workingdir = self["pdg_workingdir"].evaluateString()
        self.setWorkingDir(pdg_workingdir, pdg_workingdir)

        if not self.isCallbackServerRunning():
            self.startCallbackServer()

        return True


    def onStopCook(self, cancel):
        """
        [virtual] Callback invoked by PDG when graph cook ends.
                  Can be called multiple times.

          If cancel is True there will likely be jobs still running.
          In that case the scheduler should cancel them and block until they are actually canceled.
          This is also the time to tear down any resources that are set up in onStartCook.

          - Strange, with a vary if cancel parameter this function is called in a two completely different cases.
          - I think that it will be more clean to create 2 different functions onStopCook and onCancelCook.
        """
        self._log('onStopCook: cancel = ' + str(cancel))
        self.stopCallbackServer()

        if cancel:
            self._deleteJob()

        return True


    def onSchedule(self, work_item):
        """
        [virtual] onSchedule(self, pdg.WorkItem) -> pdg.SchedulerResult

        This callback is evaluated when the given pdg.WorkItem is ready to be executed.
        """
        if len(work_item.command) == 0:
            return pdg.scheduleResult.CookSucceeded

        self._log('onSchedule input: {} - {}'.format(work_item.node.name, work_item.name))

        # Ensure directories exist and serialize the work item
        self.createJobDirsAndSerializeWorkItems(work_item)

        # Create a task:
        task = self._constructTask(work_item)

        # Check that a block for the current TOP node exists:
        if not work_item.node.name in self.job_block_name_id:
            # We need to append a new block to the job,
            # as this is the first work item of a current TOP node.
            block = self._constructBlock(work_item)
            block.tasks.append(task)
            block_id = 0

            if self.job_id is None:
                # This is probably the first onSchedule call.
                # The job was not created.
                job = self._constructJob()
                job.blocks.append(block)
                if not self._sendJob(job):
                    return pdg.scheduleResult.CookFailed

            else:
                # Append a new block to the job
                struct = af.Cmd().appendBlocks(self.job_id, [block])
                if not 'block_ids' in struct['object']:
                    self._log('Error appending block:\n' + str(struct))
                    return pdg.scheduleResult.Failed

                block_id = struct['object']['block_ids'][0]

            self.job_block_name_id[work_item.node.name] = block_id
            self.job_tasks_id_workid[block_id] = {}
            self.job_tasks_id_workid[block_id][0] = work_item.id

        else:
            # Current TOP node block exists, we need just to append the task
            block_id = self.job_block_name_id[work_item.node.name]
            struct = af.Cmd().appendTasks(self.job_id, block_id, [task])
            if not 'task_ids' in struct['object']:
                self._log('Error appending task:\n' + str(struct))
                return pdg.scheduleResult.Failed

            task_id = struct['object']['task_ids'][0]
            self.job_tasks_id_workid[block_id][task_id] = work_item.id

        return pdg.scheduleResult.Succeeded


    def onScheduleStatic(self, dependencies, dependents, ready_items):
        """
        [virtual] Called when the scheduler should process a static dependency graph.

        dependencies    _pdg.WorkItem map of dependencies
        dependents      _pdg.WorkItem map of dependents
        ready_items     _pdg.WorkItem array of work items

         - Not Supported
        """
        self._log('onScheduleStatic:')
        print('Counts:')
        print('len(dependencies) = %d' % len(dependencies))
        print('len(dependents)   = %d' % len(dependents))
        print('len(ready)        = %d' % len(ready_items))
        return


    def onTick(self):
        """
        [virtual] Called during a cook.
                  Checks on jobs in flight to see if any have finished.
        """

        # check that the job was created
        if self.job_id is None:
            # OnTick can be called before onSchedule!
            # Before onSchedule job is not created/sent
            return pdg.tickResult.SchedulerReady

        # get job progress
        job_progress = af.Cmd().getJobProgress(self.job_id)
        if job_progress is None:
            self._log('Error getting job progress.')
            return pdg.tickResult.SchedulerBusy

        job_progress = job_progress['progress']
        ids_to_del = {}

        for block_id in self.job_tasks_id_workid:
            ids_to_del[block_id] = []
            for task_id in self.job_tasks_id_workid[block_id]:
                work_item_id = self.job_tasks_id_workid[block_id][task_id]
                task_progress = job_progress[block_id][task_id]
                state = task_progress['state']

                if state.find('RUN') != -1:
                    self.onWorkItemStartCook(work_item_id, -1)

                elif state.find('ERR') != -1:
                    if self['report_fail_on_error'].evaluateInt():
                        self.onWorkItemFailed(work_item_id, -1)
                        # If the graph is setup to block on failures, then
                        # we continue to track the task
                        if not self.isWaitForFailures:
                            ids_to_del[block_id].append(task_id)

                elif state.find('SKP') != -1:
                    self.onWorkItemCanceled(work_item_id, -1)
                    ids_to_del[block_id].append(task_id)

                elif state.find('DON') != -1:
                    time_started = task_progress['tst']
                    time_done = task_progress['tdn']
                    cook_time = float(time_started - time_done)
                    self.onWorkItemSucceeded(work_item_id, -1, cook_time)
                    ids_to_del[block_id].append(task_id)

        for block_id in ids_to_del:
            for task_id in ids_to_del[block_id]:
                del self.job_tasks_id_workid[block_id][task_id]

        return pdg.tickResult.SchedulerReady


    def submitAsJob(self, graph_file, node_path):
        """
        [virtual]
        Called when the scheduler should cook the entire TOP Network as a standalone job.
        by pressing the 'Submit as Job' button on the scheduler node UI.
        Creates a job which cooks that TOP graph using hython.
        Returns the status URI for the submitted job - just to open manager Web GUI.

        graph_file      Path to a .hip file containing the TOP Network, relative to $PDG_DIR.
        node_path       Op path to the TOP Network
        """
        self._log("submitAsJob({},{})".format(graph_file, node_path))

        # Constuct a command for hython + topcook script
        cmd = 'hython'
        # Use PDG licence
        cmd += ' --pdg'
        # Specify script that cooks graph
        cmd += ' "%s/pdgjob/topcook.py"' % os.getenv('HHP')
        # Set verbosity level
        cmd += ' --verbosity 2'
        # Set hip file:
        cmd += ' --hip "%s"' % hou.hipFile.path()
        # Set top network to cook
        cmd += ' --toppath "%s"' % node_path

        if "REZ_USED_RESOLVE" in os.environ:
            cmd = 'rez-env {} -- {}'.format(
                    os.environ["REZ_USED_RESOLVE"],
                    cmd
                )

        # Constuct a job:
        job = af.Job(self['gj_name'].evaluateString())
        job.setBranch(self['job_branch'].evaluateString())
        job.setPriority(self['gj_priority'].evaluateInt())
        job.setDependMask(self['gj_depend_mask'].evaluateString())
        job.setDependMaskGlobal(self['gj_depend_mask_global'].evaluateString())
        job.setHostsMask(self['gj_hosts_mask'].evaluateString())
        job.setHostsMaskExclude(self['gj_hosts_mask_exclude'].evaluateString())
        if self['gj_start_paused'].evaluateInt():
            job.setPaused()
        # Block
        block = af.Block('PDG-GRAPH', self['gj_service'].evaluateString())
        block.setCapacity(self['gj_capacity'].evaluateInt())
        block.addTicket(self['gj_ticket'].evaluateString(), 1)
        # Task
        task = af.Task(node_path)
        task.setCommand(cmd)
        task.setEnv('AF_USERNAME', cgruconfig.VARS['USERNAME'])
        # Append task and block and send job
        block.tasks.append(task)
        job.blocks.append(block)
        status, struct = job.send()
        print(status, struct)
        job_uri = ''
        job_id = 0
        if status and struct and 'id' in struct:
            job_id = struct['id']

        return job_uri, job_id


# Register Afanasy Scheduler type
def registerTypes(type_registry):
    type_registry.registerScheduler(AfanasyScheduler, label="Afanasy Scheduler")
