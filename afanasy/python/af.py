#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import re
import sys
import time

import cgruconfig
import cgrupathmap

import afcommon
import afnetwork
import services

Pathmap = cgrupathmap.PathMap()


def checkRegExp(pattern):
    """Missing DocString

    :param pattern:
    :return:
    """
    if len(pattern) == 0:
        return False

    result = True
    try:
        re.compile(pattern)
    except re.error:
        print('Error: Invalid regular expression pattern "%s"' % pattern)
        print(str(sys.exc_info()[1]))
        result = False
    return result


def checkClass(name, folder):
    """Missing DocString

    :param name:
    :param folder:
    :return:
    """
    filename = name + '.py'
    path = os.path.join(cgruconfig.VARS['AF_ROOT'], 'python')
    path = os.path.join(path, folder)
    if filename in os.listdir(path):
        return True
    return False


class Task:
    """Missing DocString
    """

    def __init__(self, taskname=''):
        self.data = dict()
        self.setName(taskname)

    def setName(self, name):
        """Missing DocString

        :param name:
        :return:
        """
        if name != '':
            self.data["name"] = name

    def setCommand(self, command, TransferToServer=True):
        """Missing DocString

        :param command:
        :param TransferToServer:
        :return:
        """
        if TransferToServer:
            command = Pathmap.toServer(command)
        self.data["command"] = command

    def setFiles(self, files, TransferToServer=True):
        """Missing DocString

        :param files:
        :param TransferToServer:
        :return:
        """
        if "files" not in self.data:
            self.data["files"] = []

        for afile in files:
            if TransferToServer:
                afile = Pathmap.toServer(afile)
            self.data["files"].append(afile)


class Block:
    """Missing DocString
    """

    def __init__(self, blockname='block', service='generic'):
        self.data = dict()
        self.data['flags'] = 0
        self.data["name"] = blockname
        self.data["service"] = cgruconfig.VARS['af_task_default_service']
        self.data["capacity"] = int(
            cgruconfig.VARS['af_task_default_capacity'])
        self.data["working_directory"] = Pathmap.toServer(
            os.getenv('PWD', os.getcwd()))
        # self.data["numeric"] = False
        self.tasks = []
        if service is not None and len(service):
            if self.setService(service):
                __import__("services", globals(), locals(), [self.data["service"]])
                parser = eval(('services.%s.parser') % self.data["service"])
                self.setParser(parser)

    def setService(self, service, nocheck=False):
        """Missing DocString

        :param service:
        :param nocheck:
        :return:
        """
        if service is not None and len(service):
            result = True
            if not nocheck:
                if not checkClass(service, 'services'):
                    print('Error: Unknown service "%s", setting to "generic"' %
                          service)
                    service = 'generic'
                    result = False
            self.data["service"] = service
            return result
        return False

    def setParser(self, parser, nocheck=False):
        """Missing DocString

        :param parser:
        :param nocheck:
        :return:
        """
        if parser is not None and len(parser):
            if not nocheck:
                if not checkClass(parser, 'parsers'):
                    if parser != 'none':
                        print('Error: Unknown parser "%s", setting to "none"' %
                              parser)
                        parser = 'none'
            self.data["parser"] = parser

    def setNumeric(self, start=1, end=1, pertask=1, increment=1):
        """Missing DocString

        :param int start:
        :param int end:
        :param int pertask:
        :param int increment:
        :return:
        """
        if len(self.tasks):
            print('Error: Block.setNumeric: Block already has tasks.')
            return
        if end < start:
            print('Error: Block.setNumeric: end < start (%d < %d)' % (end, start))
            end = start
        if pertask < 1:
            print('Error: Block.setNumeric: pertask < 1 (%d < 1)' % pertask)
            pertask = 1
        # self.data["numeric"] = True
        self.data['flags'] = afcommon.setBlockFlag(self.data['flags'], 'numeric')
        self.data["frame_first"] = start
        self.data["frame_last"] = end
        self.data["frames_per_task"] = pertask
        self.data["frames_inc"] = increment

    def setFramesPerTask(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        self.data["frames_per_task"] = value

    def setSequential(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        self.data["sequential"] = value

    def setCapacity(self, capacity):
        """Missing DocString

        :param capacity:
        :return:
        """
        if capacity > 0:
            self.data["capacity"] = capacity

    def setVariableCapacity(self, capacity_coeff_min, capacity_coeff_max):
        """Missing DocString

        :param capacity_coeff_min:
        :param capacity_coeff_max:
        :return:
        """
        if capacity_coeff_min >= 0 or capacity_coeff_max >= 0:
            self.data["capacity_coeff_min"] = capacity_coeff_min
            self.data["capacity_coeff_max"] = capacity_coeff_max

    def setWorkingDirectory(self, working_directory, TransferToServer=True):
        """Missing DocString

        :param working_directory:
        :param TransferToServer:
        :return:
        """
        if TransferToServer:
            working_directory = Pathmap.toServer(working_directory)
        self.data["working_directory"] = working_directory

    def setCommand(self, command, prefix=True, TransferToServer=True):
        """Missing DocString

        :param command:
        :param prefix:
        :param TransferToServer:
        :return:
        """
        if prefix:
            command = \
                os.getenv('AF_CMD_PREFIX',
                          cgruconfig.VARS['af_cmdprefix']) + command
        if TransferToServer:
            command = Pathmap.toServer(command)
        self.data["command"] = command

    def setCmdPre(self, command_pre, TransferToServer=True):
        """Missing DocString

        :param command_pre:
        :param TransferToServer:
        :return:
        """
        if TransferToServer:
            command_pre = Pathmap.toServer(command_pre)
        self.data["command_pre"] = command_pre

    def setCmdPost(self, command_post, TransferToServer=True):
        """Missing DocString

        :param command_post:
        :param TransferToServer:
        :return:
        """
        if TransferToServer:
            command_post = Pathmap.toServer(command_post)
        self.data["command_post"] = command_post

    def setFiles(self, files, TransferToServer=True):
        """Missing DocString

        :param files:
        :param TransferToServer:
        :return:
        """
        if "files" not in self.data:
            self.data["files"] = []

        for afile in files:
            if TransferToServer:
                afile = Pathmap.toServer(afile)
            self.data["files"].append(afile)

    def setEnv(self, i_name, i_value):
        """Missing DocString

        :param i_name:
        :param i_value:
        :return:
        """
        if "environment" not in self.data:
            self.data["environment"] = dict()

        self.data["environment"][i_name] = i_value

    def setName(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        self.data["name"] = value

    def setTasksName(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        self.data["tasks_name"] = value

    def setParserCoeff(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        self.data["parser_coeff"] = value

    def setErrorsAvoidHost(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        self.data["errors_avoid_host"] = value

    def setErrorsForgiveTime(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        self.data["errors_forgive_time"] = value

    def setErrorsRetries(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        self.data["errors_retries"] = value

    def setErrorsTaskSameHost(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        self.data["errors_task_same_host"] = value

    def setNeedHDD(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        self.data["need_hdd"] = value

    def setNeedMemory(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        self.data["need_memory"] = value

    def setNeedPower(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        self.data["need_power"] = value

    def setDependSubTask(self):
        """Missing DocString
        """
        self.data['flags'] = afcommon.setBlockFlag(self.data['flags'], 'dependsubtask')

    def setTasksMaxRunTime(self, value): self.setTaskMaxRunTime(value)
    def setTaskMaxRunTime(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if value > 0:
            self.data["task_max_run_time"] = value

    def setTaskMinRunTime(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if value > 0:
            self.data["task_min_run_time"] = value

    def setTaskProgressChangeTimeout(self, value):
        """If running task will not change its progress (percentage)
            for this time, it will be stopped with an error.
        :param value: timeout in seconds
        """
        if value > 0:
            self.data["task_progress_change_timeout"] = value

    def setMaxRunningTasks(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if value >= 0:
            self.data["max_running_tasks"] = value

    def setMaxRunTasksPerHost(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if value >= 0:
            self.data["max_running_tasks_per_host"] = value

    def setHostsMask(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if checkRegExp(value):
            self.data["hosts_mask"] = value

    def setHostsMaskExclude(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if checkRegExp(value):
            self.data["hosts_mask_exclude"] = value

    def setDependMask(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if checkRegExp(value):
            self.data["depend_mask"] = value

    def setTasksDependMask(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if checkRegExp(value):
            self.data["tasks_depend_mask"] = value

    def setNeedProperties(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if checkRegExp(value):
            self.data["need_properties"] = value

    def skipThumbnails(self):
        self.data['flags'] = afcommon.setBlockFlag(self.data['flags'], 'skipthumbnails')

    def skipExistingFiles(self, i_size_min=-1, i_size_max=-1):
        self.data['flags'] = afcommon.setBlockFlag(self.data['flags'], 'skipexistingfiles')
        if i_size_min != -1:
            self.data['file_size_min'] = i_size_min
        if i_size_min != -1:
            self.data['file_size_max'] = i_size_max

    def checkRenderedFiles(self, i_size_min=-1, i_size_max=-1):
        self.data['flags'] = afcommon.setBlockFlag(self.data['flags'], 'checkrenderedfiles')
        if i_size_min != -1:
            self.data['file_size_min'] = i_size_min
        if i_size_min != -1:
            self.data['file_size_max'] = i_size_max

    def setMultiHost(self, h_min, h_max, h_max_wait, master_on_slave=False,
                     service=None, service_wait=-1):
        """Missing DocString

        :param h_min:
        :param h_max:
        :param h_max_wait:
        :param master_on_slave:
        :param service:
        :param service_wait:
        :return:
        """
        if h_min < 1:
            print('Error: Block::setMultiHost: Minimum must be greater then '
                  'zero.')
            return False

        if h_max < h_min:
            print('Block::setMultiHost: Maximum must be greater or equal then '
                  'minimum.')
            return False

        if master_on_slave and service is None:
            print('Error: Block::setMultiHost: Master in slave is enabled but '
                  'service was not specified.')
            return False

        self.data['flags'] = afcommon.setBlockFlag(self.data['flags'], 'multihost')
        self.data['multihost_min'] = h_min
        self.data['multihost_max'] = h_max
        self.data['multihost_max_wait'] = h_max_wait

        if master_on_slave:
            self.data['flags'] = afcommon.setBlockFlag(self.data['flags'], 'masteronslave')

        if service:
            self.data['multihost_service'] = service

        if service_wait > 0:
            self.data['multihost_service_wait'] = service_wait

    def setSlaveLostIgnore(self):
        if not afcommon.checkBlockFlag(self.data['flags'], 'multihost'):
            print('Block::setSlaveLostIgnore: Block is not multihost.')
            return
        self.data['flags'] = afcommon.setBlockFlag(self.data['flags'], 'slavelostignore')

    def fillTasks(self):
        """Missing DocString
        """
        if len(self.tasks):
            self.data["tasks"] = []
            for task in self.tasks:
                self.data["tasks"].append(task.data)


class Job:
    """Missing DocString

    :param jobname:
    """

    def __init__(self, jobname=None):
        self.data = dict()
        self.data["name"] = "noname"
        self.data["user_name"] = cgruconfig.VARS['USERNAME']
        self.data["host_name"] = cgruconfig.VARS['HOSTNAME']
        self.data["priority"] = cgruconfig.VARS['af_priority']
        self.data["time_creation"] = int(time.time())
        self.setName(jobname)
        self.blocks = []

    def setName(self, name):
        """Missing DocString

        :param name:
        :return:
        """
        if name is not None and len(name):
            self.data["name"] = name

    def setUserName(self, username):
        """Missing DocString

        :param username:
        :return:
        """
        if username is not None and len(username):
            self.data["user_name"] = username.lower()

    def setPriority(self, priority):
        """Missing DocString

        :param priority:
        :return:
        """
        if priority < 0:
            return

        if priority > 250:
            priority = 250
            print('Warning: priority clamped to maximum = %d' % priority)

        self.data["priority"] = priority

    def setCmdPre(self, command, TransferToServer=True):
        """Missing DocString

        :param command:
        :param TransferToServer:
        :return:
        """
        if TransferToServer:
            command = Pathmap.toServer(command)
        self.data["command_pre"] = command

    def setCmdPost(self, command, TransferToServer=True):
        """Missing DocString

        :param command:
        :param TransferToServer:
        :return:
        """
        if TransferToServer:
            command = Pathmap.toServer(command)
        self.data["command_post"] = command

    def setPostDeleteFiles(self, i_path, TransferToServer=True):
        """Missing DocString

        :param command:
        :param TransferToServer:
        :return:
        """
        self.setCmdPost('deletefiles "%s"' % i_path, TransferToServer)

    def setFolder(self, i_name, i_folder, i_transferToServer=True):
        """Missing DocString

        :param i_name:
        :param i_folder:
        :param i_transferToServer:
        :return:
        """
        if i_transferToServer:
            i_folder = Pathmap.toServer(i_folder)

        if "folders" not in self.data:
            self.data["folders"] = dict()

        self.data["folders"][i_name] = i_folder

    def setPools(self, i_pools):
        self.data['pools'] = i_pools

    def setBranch(self, i_branch, i_transferToServer=True):
        if i_transferToServer:
            i_branch = Pathmap.toServer(i_branch)
        self.data['branch'] = i_branch

    def fillBlocks(self):
        """Missing DocString

        :return:
        """
        self.data["blocks"] = []
        for block in self.blocks:
            block.fillTasks()
            self.data["blocks"].append(block.data)

    def output(self):
        """Missing DocString

        :return:
        """
        self.fillBlocks()
        print(json.dumps(self.data, sort_keys=True, indent=4))

    def send(self, verbose=False):
        """Missing DocString

        :param verbose:
        :return:
        """
        if len(self.blocks) == 0:
            print('Error: Job has no blocks')

        self.fillBlocks()

        # Set folder if empty:
        if "folders" not in self.data:
            self.data["folders"] = dict()
            # Try to set output folder from files:
            for block in self.blocks:
                if "files" in block.data and len(block.data["files"]):
                    self.data["folders"][block.data['name']] = os.path.dirname(block.data["files"][0])

        # Set branch if empty:
        if 'branch' not in self.data:
            if 'output' in self.data['folders']:
                self.data['branch'] = self.data['folders']['output']
            elif 'input' in self.data['folders']:
                self.data['branch'] = self.data['folders']['input']
            else:
                self.data['branch'] = self.blocks[0].data['working_directory']

        obj = {"job": self.data}
        # print(json.dumps( obj))

        return afnetwork.sendServer(json.dumps(obj), verbose)

    def setAnnotation(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        self.data["annotation"] = value

    def setDescription(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        self.data["description"] = value

    def setProject(self, project):
        """Set the name of the project to which this job is related.

        :param project: Name of the project
        """
        if project is not None and len(project):
            self.data["project"] = project

    def setDepartment(self, department):
        """Set the name of the department which submitted this job.

        :param department: Name of the department (compositing, modeling, etc.)
        """
        if department is not None and len(department):
            self.data["department"] = department

    def setWaitTime(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if value > time.time():
            self.data["time_wait"] = int(value)

    def setMaxRunningTasks(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if value >= 0:
            self.data["max_running_tasks"] = value

    def setMaxRunTasksPerHost(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if value >= 0:
            self.data["max_running_tasks_per_host"] = value

    def setHostsMask(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if checkRegExp(value):
            self.data["hosts_mask"] = value

    def setHostsMaskExclude(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if checkRegExp(value):
            self.data["hosts_mask_exclude"] = value

    def setDependMask(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if checkRegExp(value):
            self.data["depend_mask"] = value

    def setDependMaskGlobal(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if checkRegExp(value):
            self.data["depend_mask_global"] = value

    def setNeedOS(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if checkRegExp(value):
            self.data["need_os"] = value

    def setNeedProperties(self, value):
        """Missing DocString

        :param value:
        :return:
        """
        if checkRegExp(value):
            self.data["need_properties"] = value

    def setNativeOS(self):
        """Missing DocString
        """
        self.data["need_os"] = cgruconfig.VARS['platform'][-1]

    def setAnyOS(self):
        """Missing DocString
        """
        self.data["need_os"] = ''

    def setPPApproval(self):
        self.data["ppa"] = True

    def setMaintenance(self):
        self.data["maintenance"] = True

    def setIgnoreNimby(self):
        self.data["ignorenimby"] = True

    def setIgnorePaused(self):
        self.data["ignorepaused"] = True

    def pause(self):
        """Missing DocString
        """
        self.data["offline"] = True

    def setPaused(self):
        """Missing DocString
        """
        self.data["offline"] = True

    def setOffline(self):
        """Missing DocString
        """
        self.data["offline"] = True

    def offline(self):
        """Missing DocString
        """
        self.data["offline"] = True

    def offLine(self):
        """Missing DocString
        """
        self.data["offline"] = True

    def setTimeLife(self, value):
        """Set job's time-life after which it will automatically be deleted.

        :param value: time in seconds
        """
        # this will only pass positive int
        if str(value).isdigit():
            self.data['time_life'] = value


class Cmd:
    """Missing DocString
    """

    def __init__(self):
        self.data = dict()
        self.data['user_name'] = cgruconfig.VARS['USERNAME']
        self.data['host_name'] = cgruconfig.VARS['HOSTNAME']
        self.action = None

    def _sendRequest(self, verbose=False, without_answer=False):
        """Missing DocString

        :param bool verbose:
        :return:
        """
        if self.action is None:
            print('ERROR: Action is not set.')
            return None

        obj = {self.action: self.data}
        # print(json.dumps( obj))
        output = afnetwork.sendServer(json.dumps(obj), verbose, without_answer)
        self.__init__()
        if output[0] is True:
            return output[1]
        else:
            return None

    def getJobList(self, verbose=False, ids=None):
        """Missing DocString

        :param bool verbose:
        :return:
        """
        self.action = 'get'
        self.data['type'] = 'jobs'
        if ids is not None:
            self.data['ids'] = ids
        data = self._sendRequest(verbose)
        if data is not None:
            if 'jobs' in data:
                return data['jobs']
        return None

    def deleteJob(self, jobName, verbose=False):
        """Missing DocString

        :param str jobName:
        :param bool verbose:
        :return:
        """
        self.action = 'action'
        self.data['type'] = 'jobs'
        self.data['mask'] = jobName
        self.data['operation'] = {'type': 'delete'}
        return self._sendRequest(verbose)

    def deleteJobById(self, jobId, verbose=False):
        """Missing DocString

        :param str jobName:
        :param bool verbose:
        :return:
        """
        self.action = 'action'
        self.data['type'] = 'jobs'
        self.data['ids'] = [jobId]
        self.data['operation'] = {'type': 'delete'}
        return self._sendRequest(verbose)

    def setJobState(self, jobId, state, verbose=False):
        """Missing DocString

        :param jobId:
        :param str state:
        :param bool verbose:
        :return:
        """
        self.action = 'action'
        self.data['type'] = 'jobs'
        self.data['ids'] = [jobId]
        self.data['operation'] = {'type': state}
        return self._sendRequest(verbose)

    def getJobInfo(self, jobId, verbose=False):
        """Missing DocString

        :param jobId:
        :param bool verbose:
        :return:
        """
        self.data['ids'] = [jobId]
        self.data['mode'] = 'full'
        return self.getJobList(verbose)

    def getJobProgress(self, jobId, verbose=False):
        """Missing DocString

        :param jobId:
        :param bool verbose:
        :return:
        """
        self.data['ids'] = [jobId]
        self.data['mode'] = 'progress'
        self.action = 'get'
        self.data['type'] = 'jobs'
        data = self._sendRequest(verbose)
        if data is not None:
            if 'job_progress' in data:
                return data['job_progress']
        return None

    def setBlockState(self, jobId, blockNum, state, taskIds=[], verbose=False):
        """Missing DocString

        :param jobId:
        :param blockNum:
        :param str state:
        :param bool verbose:
        :return:
        """
        self.action = 'action'
        self.data['type'] = 'jobs'
        self.data['ids'] = [jobId]
        self.data['block_ids'] = [blockNum]
        self.data['operation'] = {'type': state,
                                  'task_ids': taskIds}
        return self._sendRequest(verbose)

    def renderSetUserName(self, i_user_name):
        """Missing DocString

        :param i_name:
        :return:
        """
        self.action = 'action'
        self.data['type'] = 'renders'
        self.data['mask'] = cgruconfig.VARS['HOSTNAME']
        self.data['params'] = {'user_name': i_user_name}
        self._sendRequest()

    def renderSetNimby(self):
        """Missing DocString

        :return:
        """
        self.action = 'action'
        self.data['type'] = 'renders'
        self.data['mask'] = cgruconfig.VARS['HOSTNAME']
        self.data['params'] = {'nimby': True}
        self._sendRequest()

    def renderSetNIMBY(self):
        """Missing DocString

        :return:
        """
        self.action = 'action'
        self.data['type'] = 'renders'
        self.data['mask'] = cgruconfig.VARS['HOSTNAME']
        self.data['params'] = {'NIMBY': True}
        self._sendRequest()

    def renderSetFree(self):
        """Missing DocString

        :return:
        """
        self.action = 'action'
        self.data['type'] = 'renders'
        self.data['mask'] = cgruconfig.VARS['HOSTNAME']
        self.data['params'] = {'nimby': False}
        self._sendRequest()

    def renderSetFreeUnpause(self):
        """Missing DocString

        :return:
        """
        self.action = 'action'
        self.data['type'] = 'renders'
        self.data['mask'] = cgruconfig.VARS['HOSTNAME']
        self.data['params'] = {'nimby': False, 'paused': False}
        self._sendRequest()

    def renderEjectTasks(self):
        """Missing DocString

        :return:
        """
        self.action = 'action'
        self.data['type'] = 'renders'
        self.data['mask'] = cgruconfig.VARS['HOSTNAME']
        self.data['operation'] = {'type': 'eject_tasks'}
        self._sendRequest()

    def renderEjectNotMyTasks(self):
        """Missing DocString

        :return:
        """
        self.action = 'action'
        self.data['type'] = 'renders'
        self.data['mask'] = cgruconfig.VARS['HOSTNAME']
        self.data['operation'] = {'type': 'eject_tasks_keep_my'}
        self._sendRequest()

    def renderExit(self):
        """Missing DocString

        :return:
        """
        self.action = 'action'
        self.data['type'] = 'renders'
        self.data['mask'] = cgruconfig.VARS['HOSTNAME']
        self.data['operation'] = {'type': 'exit'}
        self._sendRequest()

    def monitorExit(self):
        """Missing DocString

        :return:
        """
        self.action = 'action'
        self.data['type'] = 'monitors'
        self.data['mask'] = cgruconfig.VARS['USERNAME'] + '@' + cgruconfig.VARS['HOSTNAME'] + '.*'
        self.data['operation'] = {'type': 'exit'}
        self._sendRequest()

    def monitorRegister(self):
        """Missing DocString
        :return:
        """
        self.__init__()
        self.action = "monitor"
        self.data['engine'] = 'python'
        result = self._sendRequest()
        monitorId = result['monitor']['id']
        return monitorId

    def monitorChangeUid(self, monitorId, uid):
        """Missing DocString
        :param monitorId:
        :param uid:
        :return:
        """
        self.action = "action"
        self.data["type"] = "monitors"
        self.data["ids"] = [monitorId]
        self.data["operation"] = {"type": "watch",
                                  "class": "perm",
                                  "uid": uid}
        return self._sendRequest()

    def monitorUnregister(self, monitorId):
        """Missing DocString

        :param monitorId:
        :return:
        """
        self.action = "action"
        self.data["type"] = "monitors"
        self.data["ids"] = [monitorId]
        self.data["operation"] = {"type": "deregister"}
        return self._sendRequest(without_answer=True)

    def monitorSubscribe(self, monitorId, classType):
        """Missing DocString

        :param monitorId:
        :param classType:
        :return:
        """
        self.action = "action"
        self.data["type"] = "monitors"
        self.data["ids"] = [monitorId]
        self.data["operation"] = {"type": "watch",
                                  "class": classType,
                                  "status": "subscribe"}
        return self._sendRequest()

    def monitorEvents(self, monitorId):
        """Missing DocString

        :param monitorId:
        :return:
        """
        self.action = 'get'
        self.data['type'] = 'monitors'
        self.data['ids'] = [monitorId]
        self.data['mode'] = 'events'
        return self._sendRequest()

    def renderGetList(self, mask=None):
        """Missing DocString

        :param mask:
        :return:
        """
        self.action = 'get'
        self.data['type'] = 'renders'
        if mask is not None:
            self.data['mask'] = mask
        data = self._sendRequest()
        if data is not None:
            if 'renders' in data:
                return data['renders']
        return None

    def renderGetId(self, i_id, i_mode=None):
        """Missing DocString
        :param i_id:
        :param i_mode:
        :return:
        """
        self.action = 'get'
        self.data['type'] = 'renders'
        self.data['ids'] = [i_id]
        if i_mode is not None:
            self.data['mode'] = i_mode
        data = self._sendRequest()
        return data
        if data is not None:
            if 'renders' in data:
                return data['renders']
        return None

    def renderGetResources(self):
        """Missing DocString

        :return:
        """
        self.action = 'get'
        self.data['type'] = 'renders'
        self.data['mode'] = 'resources'
        data = self._sendRequest()
        if data is not None:
            if 'renders' in data:
                return data['renders']
        return None

    def renderGetLocal(self):
        """Missing DocString

        :return:
        """
        return self.renderGetList(cgruconfig.VARS['HOSTNAME'])

    def appendBlocks(self, jobId, blocks, verbose=False):
        """Append new blocks to an existing job

        :param jobId: Id of the job to which blocks are added
        :param blocks: list of new Block() objects
        :param bool verbose: verbosity toggle
        :return: server response
        """
        blocks_data = []
        for b in blocks:
            b.fillTasks()
            blocks_data.append(b.data)

        self.action = 'action'
        self.data['type'] = 'jobs'
        self.data['ids'] = [jobId]
        self.data['operation'] = {'type': 'append_blocks',
                                  'blocks': blocks_data}
        return self._sendRequest(verbose)

    def appendTasks(self, jobId, blockId, tasks, verbose=False):
        """Append new tasks to an existing block

        :param jobId: Id of the job to which tasks are added
        :param blockId: Index of the block to which tasks are added
        :param blocks: list of new Task() objects
        :param bool verbose: verbosity toggle
        :return: server response
        """
        tasks_data = []
        for t in tasks:
            tasks_data.append(t.data)

        self.action = 'action'
        self.data['type'] = 'jobs'
        self.data['ids'] = [jobId]
        self.data['block_ids'] = [blockId]
        self.data['operation'] = {'type': 'append_tasks',
                                  'tasks': tasks_data}
        return self._sendRequest(verbose)
