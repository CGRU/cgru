#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import re
import sys
import time

import cgruconfig
import cgrupathmap
import afnetwork
import services

Pathmap = cgrupathmap.PathMap()

def checkRegExp( pattern):
	result = True
	try:
		re.compile(pattern)
	except:
		print('Error: Invalid regular expression pattern "%s"' % pattern)
		print( str(sys.exc_info()[1]))
		result = False
	return result

def checkClass( name, folder):
	filename = name + '.py'
	path = os.path.join( cgruconfig.VARS['AF_ROOT'], 'python')
	path = os.path.join( path, folder)
	if filename in os.listdir( path): return True
	return False

class Task:
	def __init__( self, taskname = ''):
		self.data = dict()
		self.setName( taskname)

	def setName( self, name):
		if name != '': self.data["name"] = name

	def setCommand( self, command, TransferToServer = True):
		if TransferToServer: command = Pathmap.toServer( command)
		self.data["command"] = command

	def setFiles( self, files, TransferToServer = True):
		if TransferToServer: files = Pathmap.toServer( files)
		self.data["files"] = files

class Block:
	def __init__( self, blockname = 'block', service = 'generic'):
		self.data = dict()
		self.data["name"] = blockname
		self.data["capacity"] = int( cgruconfig.VARS['af_task_default_capacity'])
		self.data['working_directory'] = Pathmap.toServer( os.getenv('PWD', os.getcwd()))
		self.tasks = []
		if self.setService( service):
			__import__("services", globals(), locals(), [self.data["service"]])
			parser = eval(('services.%s.parser') % self.data["service"])
			self.setParser( parser)

	def setService( self, service, nocheck = False):
		if service != '':
			result = True
			if not nocheck:
				if not checkClass( service, 'services'):
					print('Error: Unknown service "%s", setting to "generic"' % service)
					service = 'generic'
					result = False
			self.data["service"] = service
			return result
		return False

	def setParser( self, parser, nocheck = False):
		if parser != '':
			if not nocheck:
				if not checkClass( parser, 'parsers'):
					if parser != 'none':
						print('Error: Unknown parser "%s", setting to "none"' % parser)
						parser = 'none'
		self.data["parser"] = parser

	def setNumeric( self, start = 1, end = 10, pertask = 1, increment = 1):
		if len( self.tasks):
			print('Error: Block.setNumeric: Block already has tasks.')
			return
		if end < start:
			print('Error: Block.setNumeric: end < start (%d < %d)' % (end,start))
			end = start
		if pertask < 1:
			print('Error: Block.setNumeric: pertask < 1 (%d < 1)' % pertask)
			pertask = 1
		self.data["frame_first"] = start
		self.data["frame_last"] = end
		self.data["frames_per_task"] = pertask
		self.data["frames_inc"] = increment

	def setFlags(self, flags):
		self.data['flags'] = flags

	def setCapacity( self, capacity):
		if capacity > 0:
			self.data["capacity"] = capacity

	def setVariableCapacity( self, capacity_coeff_min, capacity_coeff_max):
		if capacity_coeff_min >= 0 or capacity_coeff_max >= 0:
			self.data["capacity_coeff_min"] = capacity_coeff_min
			self.data["capacity_coeff_max"] = capacity_coeff_max

	def setWorkingDirectory( self, working_directory, TransferToServer = True):
		if TransferToServer: working_directory = Pathmap.toServer( working_directory)
		self.data["working_directory"] = working_directory

	def setCommand( self, command, prefix = True, TransferToServer = True):
		if prefix: command = os.getenv('AF_CMD_PREFIX', cgruconfig.VARS['af_cmdprefix']) + command
		if TransferToServer: command = Pathmap.toServer( command)
		self.data["command"] = command

	def setCmdPre(  self, command_pre, TransferToServer = True):
		if TransferToServer: command_pre = Pathmap.toServer( command_pre)
		self.data["command_pre"] = command_pre

	def setCmdPost( self, command_post, TransferToServer = True):
		if TransferToServer: command_post = Pathmap.toServer( command_post)
		self.data["command_post"] = command_post

	def setFiles(  self, files, TransferToServer = True):
		if TransferToServer: files = Pathmap.toServer( files)
		self.data["files"] = files

	def setName(               self, value): self.data["name"] = value
	def setTasksName(          self, value): self.data["tasks_name"] = value
	def setFramesPerTask(      self, value): self.data["frames_per_task"] = value
	def setParserCoeff(        self, value): self.data["parser_coeff"] = value
	def setErrorsAvoidHost(    self, value): self.data["errors_avoid_host"] = value
	def setErrorsForgiveTime(  self, value): self.data["errors_forgive_time"] = value
	def setErrorsRetries(      self, value): self.data["errors_retries"] = value
	def setErrorsTaskSameHost( self, value): self.data["errors_task_same_host"] = value
	def setNeedHDD(            self, value): self.data["need_hdd"] = value
	def setNeedMemory(         self, value): self.data["need_memory"] = value
	def setNeedPower(          self, value): self.data["need_power"] = value
	def setNonSequential( self, value = True ): self.data["non_sequential"] = value
	def setDependSubTask( self, value = True ): self.data["depend_sub_task"] = value
	def setTasksMaxRunTime(    self, value):
		if( value > 0 ): self.data["tasks_max_run_time"] = value
	def setMaxRunningTasks(    self, value):
		if( value >= 0 ): self.data["max_running_tasks"] = value
	def setMaxRunTasksPerHost( self, value):
		if( value >= 0 ): self.data["max_running_tasks_per_host"] = value

	def setHostsMask(        self, value):
		if checkRegExp(value): self.data["hosts_mask"] = value
	def setHostsMaskExclude( self, value):
		if checkRegExp(value): self.data["hosts_mask_exclude"] = value
	def setDependMask(       self, value):
		if checkRegExp(value): self.data["depend_mask"] = value
	def setTasksDependMask( self, value):
		if checkRegExp(value): self.data["tasks_depend_mask"] = value
	def setNeedProperties(   self, value):
		if checkRegExp(value): self.data["need_properties"] = value

	def setGenThumbnails(self, value = True): self.data["gen_thumbnails"] = value;
	
	def setDoPost( self, value = True ): self.data["do_post"] = value
	
	def setMultiHost( self, h_min, h_max, h_max_wait, master_on_slave = False, service = None, service_wait = -1):
		if h_min < 1:
			print('Error: Block::setMultiHost: Minimum must be greater then zero.')
			return False
		if h_max < h_min:
			print('Block::setMultiHost: Maximum must be greater or equal then minimum.')
			return False
		if master_on_slave and service is None:
			print('Error: Block::setMultiHost: Master in slave is enabled but service was not specified.')
			return False

		self.data['multihost_min'] = h_min
		self.data['multihost_max'] = h_max
		self.data['multihost_max_wait'] = h_max_wait
		if master_on_slave:
			self.data['multihost_master_on_slave'] = master_on_slave
		if service:
			self.data['multihost_service'] = service
		if service_wait > 0:
			self.data['multihost_service_wait'] = service_wait


	def fillTasks( self):
		if len( self.tasks):
			self.data["tasks"] = []
			for task in self.tasks:
				self.data["tasks"].append( task.data)

class Job:
	def __init__( self, jobname = None, verbose = False):
		self.data = dict()
		self.data["name"] = "noname"
		self.data["user_name"] = cgruconfig.VARS['USERNAME']
		self.data["host_name"] = cgruconfig.VARS['HOSTNAME']
		self.data["priority"]  = cgruconfig.VARS['af_priority']
		self.data["time_creation"] = int(time.time())
		self.setName( jobname)
		self.blocks = []

	def setName( self, name):
		if name is not None and len(name):
			self.data["name"] = name

	def setUserName( self, username):
		if username is not None and len(username):
			self.data["user_name"] = username.lower()

	def setPriority( self, priority):
		if priority < 0: return
		if priority > 250:
			priority = 250
			print('Warning: priority clamped to maximum = %d' % priority)
		self.data["priority"] = priority

	def setCmdPre(  self, command, TransferToServer = True):
		if TransferToServer: command = Pathmap.toServer( command)
		self.data["command_pre"] = command

	def setCmdPost( self, command, TransferToServer = True):
		if TransferToServer: command = Pathmap.toServer( command)
		self.data["command_post"] = command

	def fillBlocks( self):
		self.data["blocks"] = []
		for block in self.blocks:
			block.fillTasks()
			self.data["blocks"].append( block.data)

	def output( self, full = False):
		self.fillBlocks()
		print( json.dumps( self.data, sort_keys=True, indent=4))

	def send( self, verbose = False):
		if len( self.blocks) == 0:
			print('Error: Job has no blocks')
			return False
		self.fillBlocks()

		obj = {"job": self.data }
		#print(json.dumps( obj))

		return afnetwork.sendServer( json.dumps( obj), False, verbose)[0]

	def setAnnotation(         self, value): self.data["annotation"] = value
	def setDescription(        self, value): self.data["description"] = value
	def setWaitTime(           self, value):
		if( value > 0 ): self.data["time_wait"] = value
	def setMaxRunningTasks(    self, value):
		if( value >= 0 ): self.data["max_running_tasks"] = value
	def setMaxRunTasksPerHost( self, value):
		if( value >= 0 ): self.data["max_running_tasks_per_host"] = value

	def setHostsMask(        self, value):
		if checkRegExp(value): self.data["hosts_mask"] = value
	def setHostsMaskExclude( self, value):
		if checkRegExp(value): self.data["hosts_mask_exclude"] = value
	def setDependMask(       self, value):
		if checkRegExp(value): self.data["depend_mask"] = value
	def setDependMaskGlobal( self, value):
		if checkRegExp(value): self.data["depend_mask_global"] = value
	def setNeedOS(           self, value):
		if checkRegExp(value): self.data["need_os"] = value
	def setNeedProperties(   self, value):
		if checkRegExp(value): self.data["need_properties"] = value

	def setNativeOS( self): self.data["need_os"] = cgruconfig.VARS['platform'][-1]
	def setAnyOS(    self): self.data["need_os"] = ''

	def pause(      self): self.data["offline"] = True
	def setPaused(  self): self.data["offline"] = True
	def setOffline( self): self.data["offline"] = True
	def offline(    self): self.data["offline"] = True
	def offLine(    self): self.data["offline"] = True

class Cmd:
	def __init__( self ):
		self.data = dict()
		self.data['user_name'] = cgruconfig.VARS['USERNAME']
		self.data['host_name'] = cgruconfig.VARS['HOSTNAME']
		self.action = None
   
	def _sendRequest(self, verbose = False):
		if self.action is None:
			print('ERROR: Action is not set.')
			return None

		receive = ( self.action == 'get')
		obj = { self.action: self.data }
		#print(json.dumps( obj))
		output = afnetwork.sendServer( json.dumps( obj), receive, verbose)

		if output[0] == True:
			return output[1]
		else:
			return None

	def getJobList( self, verbose = False):
		self.action = 'get'
		self.data['type'] = 'jobs'
		data = self._sendRequest()
		if data is not None:
			if 'jobs' in data:
				return data['jobs']
		return None

	def deleteJob(self, jobName, verbose = False):
		self.action = 'action'
		self.data['type'] = 'jobs'
		self.data['mask'] = jobName
		self.data['operation'] = {'type':'delete'}
		return self._sendRequest(verbose)

	def getJobInfo(self, jobId, verbose = False):
		self.data['ids'] = [ jobId ]
		self.data['mode'] = 'full'
		return self.getJobList(verbose)

	def renderSetNimby( self, text):
		self.action = 'action'
		self.data['type'] = 'renders'
		self.data['mask'] = cgruconfig.VARS['HOSTNAME']
		self.data['params'] = {'nimby':True}
		self._sendRequest()

	def renderSetNIMBY( self, text):
		self.action = 'action'
		self.data['type'] = 'renders'
		self.data['mask'] = cgruconfig.VARS['HOSTNAME']
		self.data['params'] = {'NIMBY':True}
		self._sendRequest()

	def renderSetFree( self, text):
		self.action = 'action'
		self.data['type'] = 'renders'
		self.data['mask'] = cgruconfig.VARS['HOSTNAME']
		self.data['params'] = {'nimby':False}
		self._sendRequest()

	def renderEjectTasks( self, text):
		self.action = 'action'
		self.data['type'] = 'renders'
		self.data['mask'] = cgruconfig.VARS['HOSTNAME']
		self.data['operation'] = {'type':'eject_tasks'}
		self._sendRequest()

	def renderEjectNotMyTasks( self, text):
		self.action = 'action'
		self.data['type'] = 'renders'
		self.data['mask'] = cgruconfig.VARS['HOSTNAME']
		self.data['operation'] = {'type':'eject_tasks_keep_my'}
		self._sendRequest()

	def renderExit( self, text):
		self.action = 'action'
		self.data['type'] = 'renders'
		self.data['mask'] = cgruconfig.VARS['HOSTNAME']
		self.data['operation'] = {'type':'exit'}
		self._sendRequest()

	def talkExit( self, text):
		self.action = 'action'
		self.data['type'] = 'talks'
		self.data['mask'] = cgruconfig.VARS['USERNAME'] + '@' + cgruconfig.VARS['HOSTNAME'] + ':.*'
		self.data['operation'] = {'type':'exit'}
		self._sendRequest()

	def monitorExit( self, text):
		self.action = 'action'
		self.data['type'] = 'monitors'
		self.data['mask'] = cgruconfig.VARS['USERNAME'] + '@' + cgruconfig.VARS['HOSTNAME'] + ':.*'
		self.data['operation'] = {'type':'exit'}
		self._sendRequest()

	def renderGetList( self, mask = None):
		self.action = 'get'
		self.data['type'] = 'renders'
		if mask is not None:
			self.data['mask'] = mask
		data = self._sendRequest()
		if data is not None:
			if 'renders' in data:
				return data['renders']
		return None

	def renderGetLocal( self): return self.renderGetList( cgruconfig.VARS['HOSTNAME'])
