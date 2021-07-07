import json
import logging
import os
import sys
import traceback

import pdg
from pdg.scheduler import PyScheduler
from pdg.job.callbackserver import CallbackServerMixin
from pdg.utils import expand_vars

import af

logging.basicConfig(level = logging.DEBUG)
logger = logging.getLogger(__name__)


class AfanasyScheduler(CallbackServerMixin, PyScheduler):
    """
    Scheduler implementation that interfaces with a Afanasy farm instance.
    """
    def __init__(self, scheduler, name):
        PyScheduler.__init__(self, scheduler, name)
        CallbackServerMixin.__init__(self, False)
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
        self.job_tasks_id_name = {}


    def _deleteJob(self):
        if self.job_id is not None:
            af.Cmd().deleteJobById(self.job_id)
        self._initData()


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
        logger.debug("onStart")
        return True


    def onStop(self):
        """
        [virtual] Called by PDG when scheduler is cleaned up.
        """
        logger.debug("onStop")
        self.stopCallbackServer()
        self._deleteJob()
        return True


    def onStartCook(self, static, cook_set):
        """
        onStartCook(self, static, cook_set) -> boolean

        [virtual] Cook start callback. Starts the job for the cook session.
        """
        logger.debug("onStartCook")
        self._deleteJob()

        pdg_workingdir = self["pdg_workingdir"].evaluateString()
        self.setWorkingDir(pdg_workingdir, pdg_workingdir)

        if not self.isCallbackServerRunning():
            self.startCallbackServer()

        # Create Job
        job = af.Job(self['job_name'].evaluateString())

        # Job Parameters
        job.setBranch(self['job_branch'].evaluateString())
        job.setDependMask(self['depend_mask'].evaluateString())
        job.setDependMaskGlobal(self['depend_mask_global'].evaluateString())
        job.setPriority(self['priority'].evaluateInt())
        job.setMaxRunningTasks(self['max_runtasks'].evaluateInt())
        job.setMaxRunTasksPerHost(self['maxperhost'].evaluateInt())
        job.setHostsMask(self['hosts_mask'].evaluateString())
        job.setHostsMaskExclude(self['hosts_mask_exclude'].evaluateString())

        # Create a block
        block = af.Block('control', 'hbatch')
        block.setCapacity(self['capacity'].evaluateInt())
        block.setMaxRunningTasks(0)

        # Control task, not supported, yet, empty for now
        task = af.Task('control')
        #task.setCommand('')
        block.tasks.append(task)

        job.blocks.append(block)

        try:
            self.job_id = job.send()
            self.job_id = self.job_id[1]['id']
        except Exception,err:
            traceback.print_exc()
            sys.stderr.flush()
            raise RuntimeError('Error creating PDG job.')

        return True


    def onStopCook(self, cancel):
        """
        [virtual] Callback invoked by PDG when graph cook ends.
                  Can be called multiple times.
        """
        logger.debug("onStopCook")
        self.stopCallbackServer()

        return True


    def onSchedule(self, work_item):
        """
        [virtual] onSchedule(self, pdg.WorkItem) -> pdg.SchedulerResult

        This callback is evaluated when the given pdg.WorkItem is ready to be executed.
        """
        if len(work_item.command) == 0:
            return pdg.scheduleResult.CookSucceeded

        logger.debug('onSchedule input: {} {}'.format(work_item.node.name, work_item.name))

        # Find a node block or create a new one
        block = None
        block_id = 0
        if work_item.node.name in self.job_block_name_id:
            block_id = self.job_block_name_id[work_item.node.name]
        else:
            block = af.Block(work_item.node.name, 'hbatch_mantra')

        # Ensure directories exist and serialize the work item
        self.createJobDirsAndSerializeWorkItems(work_item)

        # Create Task:
        task = af.Task(work_item.name)
        task.setCommand(self.expandCommandTokens(work_item.command, work_item))

        # Set Environment Task Variables
        task.setEnv('PDG_RESULT_SERVER', str(self.workItemResultServerAddr()))
        task.setEnv('PDG_ITEM_NAME', str(work_item.name))
        task.setEnv('PDG_DIR', str(self.workingDir(False)))
        task.setEnv('PDG_TEMP', str(self.tempDir(False)))
        task.setEnv('PDG_SHARED_TEMP', str(self.tempDir(False)))
        task.setEnv('PDG_INDEX', str(work_item.index))
        task.setEnv('PDG_INDEX4', "{:04d}".format(work_item.index))
        task.setEnv('PDG_SCRIPTDIR', str(self.scriptDir(False)))

        # Append Task to the job:
        cmd = af.Cmd()
        if block is not None:
            block.tasks.append(task)
            struct = cmd.appendBlocks(self.job_id, [block])
            if not 'block_ids' in struct['object']:
                print("Error appending block:")
                print(struct)
                return pdg.scheduleResult.Failed

            block_id = struct['object']['block_ids'][0]
            self.job_block_name_id[work_item.node.name] = block_id
            self.job_tasks_id_name[block_id] = {}
            self.job_tasks_id_name[block_id][0] = work_item.name
        else:
            struct = cmd.appendTasks(self.job_id, block_id, [task])
            if not 'task_ids' in struct['object']:
                print("Error appending task:")
                print(struct)
                return pdg.scheduleResult.Failed

            task_id = struct['object']['task_ids'][0]
            self.job_tasks_id_name[block_id][task_id] = work_item.name

        return pdg.scheduleResult.Succeeded


    def onScheduleStatic(self, dependencies, dependents, ready_items):
        """
        [virtual] Called when the scheduler should process a static dependency graph.

        dependencies    _pdg.WorkItem map of dependencies
        dependents      _pdg.WorkItem map of dependents
        ready_items     _pdg.WorkItem array of work items

         - Not Supported
        """
        logger.debug('onScheduleStatic:')
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
            return tickResult.SchedulerCancelCook

        # get job progress
        cmd = af.Cmd()
        job_progress = cmd.getJobProgress(self.job_id)
        if job_progress is None:
            print('Error getting job progress.')
            return pdg.tickResult.SchedulerBusy

        job_progress = job_progress['progress']
        ids_to_del = {}

        for block_id in self.job_tasks_id_name:
            ids_to_del[block_id] = []
            for task_id in self.job_tasks_id_name[block_id]:
                work_item_name = self.job_tasks_id_name[block_id][task_id]
                task_progress = job_progress[block_id][task_id]
                state = task_progress['state']

                if state.find('RUN') != -1:
                    self.onWorkItemStartCook(work_item_name, -1)

                elif state.find('ERR') != -1:
                    self.onWorkItemFailed(work_item_name, -1)
                    ids_to_del[block_id].append(task_id)

                elif state.find('SKP') != -1:
                    self.onWorkItemCanceled(work_item_name, -1)
                    ids_to_del[block_id].append(task_id)

                elif state.find('DON') != -1:
                    time_started = task_progress['tst']
                    time_done = task_progress['tdn']
                    cook_time = float(time_started - time_done)
                    self.onWorkItemSucceeded(work_item_name, -1, cook_time)
                    ids_to_del[block_id].append(task_id)

        for block_id in ids_to_del:
            for task_id in ids_to_del[block_id]:
                del self.job_tasks_id_name[block_id][task_id]

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
        # not supported, yet
        logger.debug("submitAsJob({},{})".format(graph_file, node_path))
        return ""


# Register Afanasy Scheduler type
def registerTypes(type_registry):
    type_registry.registerScheduler(AfanasyScheduler, label="Afanasy Scheduler")
