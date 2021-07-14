import json
import logging
import os
import sys
import traceback

import hou
import pdg
from pdg.scheduler import PyScheduler
from pdg.job.callbackserver import CallbackServerMixin
from pdg.utils import expand_vars

import cgruconfig
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


    def _constructJob(self):
        job = af.Job(self['job_name'].evaluateString())

        job.setBranch(self['job_branch'].evaluateString())
        job.setDependMask(self['depend_mask'].evaluateString())
        job.setDependMaskGlobal(self['depend_mask_global'].evaluateString())
        job.setPriority(self['priority'].evaluateInt())
        job.setMaxRunningTasks(self['max_runtasks'].evaluateInt())
        job.setMaxRunTasksPerHost(self['maxperhost'].evaluateInt())
        job.setHostsMask(self['hosts_mask'].evaluateString())
        job.setHostsMaskExclude(self['hosts_mask_exclude'].evaluateString())

        return job


    def _constructBlock(self, work_item):
        block = af.Block(work_item.node.name, 'hbatch_mantra')
        block.setCapacity(self['capacity'].evaluateInt())
        block.setNeedMemory(self['min_memory'].evaluateInt()*1024)
        block.setTaskMinRunTime(self['minruntime'].evaluateInt())
        block.setTaskMaxRunTime(int(self['maxruntime'].evaluateFloat()*3600.0))
        # Check service and parser:
        # PDG uses "ALF_PROGRESS" everywhere
        block.setParser('mantra')

        return block


    def _constructTask(self, work_item):
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
        logger.debug("onStopCook: cancel = " + str(cancel))
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

        logger.debug('onSchedule input: {} {}'.format(work_item.node.name, work_item.name))

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
                    print("Error appending block:")
                    print(struct)
                    return pdg.scheduleResult.Failed

                block_id = struct['object']['block_ids'][0]

            self.job_block_name_id[work_item.node.name] = block_id
            self.job_tasks_id_name[block_id] = {}
            self.job_tasks_id_name[block_id][0] = work_item.name

        else:
            # Current TOP node block exists, we need just to append the task
            block_id = self.job_block_name_id[work_item.node.name]
            struct = af.Cmd().appendTasks(self.job_id, block_id, [task])
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
            # OnTick can be called before onSchedule!
            # Before onSchedule job is not created/sent
            return pdg.tickResult.SchedulerReady

        # get job progress
        job_progress = af.Cmd().getJobProgress(self.job_id)
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
        logger.debug("submitAsJob({},{})".format(graph_file, node_path))

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

        # Constuct a job:
        job = self._constructJob()
        block = af.Block('PDG-GRAPH','hbatch')
        task = af.Task(node_path)
        task.setCommand(cmd)
        task.setEnv('AF_USERNAME', cgruconfig.VARS['USERNAME'])
        block.tasks.append(task)
        job.blocks.append(block)
        job.send()

        return None


# Register Afanasy Scheduler type
def registerTypes(type_registry):
    type_registry.registerScheduler(AfanasyScheduler, label="Afanasy Scheduler")
