#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import sys
import time

import afenv
import afnetwork
import services

from afpathmap import PathMap
pathmap = PathMap()

def CheckClass( name, folder):
	filename = name + '.py'
	path = os.path.join( afenv.VARS['AF_ROOT'], 'python')
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
		if TransferToServer: command = pathmap.toServer( command)
		self.data["command"] = command

	def setFiles( self, files, TransferToServer = True):
		if TransferToServer: files = pathmap.toServer( files)
		self.data["files"] = files

class Block:
	def __init__( self, blockname = 'block', service = 'generic'):
		parser = ''
		if not CheckClass( service, 'services'):
			print('Error: Unknown service "%s", setting to "generic"' % service)
			service = 'generic'
		else:
			__import__("services", globals(), locals(), [service])
			parser = eval(('services.%s.parser') % service)

		self.data = dict()
		self.data["name"] = blockname
		self.data["service"] = service
		self.data["parser"] = parser
		self.data["capacity"] = int( afenv.VARS['task_default_capacity'])
		self.data["working_directory"] = os.getenv('PWD', os.getcwd())

	def setParser( self, parser, nocheck = False):
		if parser != '':
			if not nocheck:
				if not CheckClass( parser, 'parsers'):
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

	def setCapacity( self, capacity):
		if capacity < 0:
			print('Error: Block.setCapacity: capacity < 0 (%d < 0)' % capacity)
			return
		self.data["capacity"] = capacity

	def setVariableCapacity( self, capacity_coeff_min, capacity_coeff_max):
		if capacity_coeff_min >= 0 or capacity_coeff_max >= 0:
			self.data["capacity_coeff_min"] = capacity_coeff_min
			self.data["capacity_coeff_max"] = capacity_coeff_max

	def setWorkingDirectory( self, working_directory, TransferToServer = True):
		if TransferToServer: working_directory = pathmap.toServer( working_directory)
		self.data["working_directory"] = working_directory

	def setCommand( self, command, prefix = True, TransferToServer = True):
		if prefix: command = os.getenv('AF_CMD_PREFIX', afenv.VARS['cmdprefix']) + command
		if TransferToServer: command = pathmap.toServer( command)
		self.data["command"] = command

	def setCmdPre(  self, command_pre, TransferToServer = True):
		if TransferToServer: command_pre = pathmap.toServer( command_pre)
		self.data["command_pre"] = command_pre

	def setCmdPost( self, command_post, TransferToServer = True):
		if TransferToServer: command_post = pathmap.toServer( command_post)
		self.data["command_post"] = command_post

	def setFiles(  self, files, TransferToServer = True):
		if TransferToServer: files = pathmap.toServer( files)
		self.data["files"] = files

	def fillTasks( self):
		if len( self.tasks):
			self.data["tasks"] = []
			for task in self.tasks:
				self.data["tasks"].append( task.data)

class Job:
	def __init__( self, jobname = None, verbose = False):
		self.data = dict()
		self.data["blocks"] = []
		self.data["name"] = "noname"
		self.data["user_name"] = afenv.VARS['USERNAME']
		self.data["host_name"] = afenv.VARS['HOSTNAME']
		self.data["priority"]  = afenv.VARS['priority']
		self.data["time_creation"] = int(time.time())
		if jobname is not None: self.setName( jobname)

	def setName( self, name):
		if len(name):
			self.data["name"] = name

	def setUserName( self, username):
		if username == None or username == '':
			username = 'none'
			print('Error: Username is empty.')
		self.data["user_name"] = username.lower()

	def setPriority( self, priority):
		if priority < 0: return
		if priority > 250:
			priority = 250
			print('Warning: priority clamped to maximum = %d' % priority)
		self.data["priority"] = priority

	def setCmdPre(  self, command, TransferToServer = True):
		if TransferToServer: command = pathmap.toServer( command)
		self.data["command_pre"] = command

	def setCmdPost( self, command, TransferToServer = True):
		if TransferToServer: command = pathmap.toServer( command)
		self.data["command_post"] = command

	def fillBlocks( self):
		for block in self.blocks:
			self.data["blocks"].append( block.data)
		self.clearBlocksList()
		b = 0
		for block in self.blocks:
			if isinstance( block, Block):
				block.fillTasks()
				self.appendBlock( block)
			else:
				print('Warning: Skipping element[%d] of list "blocks" which is not an instance of "Block" class:' % b)
				print(repr(block))
			b += 1

	def output( self, full = False):
		self.fillBlocks()
		if full: print('Job information:')
		else:    print('Job: ',)
		pyaf.Job.output( self, full)

	def send( self, verbose = False):
		if len( self.blocks) == 0:
			print('Error: Job has no blocks')
			return False
		self.fillBlocks()

		obj = {"job": self.data }
		#print(json.dumps( obj))

		if self.construct() == False: return False
		return afnetwork.sendServer( json.dumps( obj), False, verbose)[0]

	def pause(      self): self.data["pause"] = true
	def setPaused(  self): self.data["pause"] = true
	def setOffline( self): self.data["pause"] = true
	def offline(    self): self.data["pause"] = true


class Cmd:
	def __init__( self ):
		self.data = dict()
		self.data['user_name'] = afenv.VARS['USERNAME']
		self.data['host_name'] = afenv.VARS['HOSTNAME']
		self.action = None
   
	def _sendRequest(self, verbose = False):
		if self.action is None:
			print('ERROR: Action is not set.')
			return None

		obj = { self.action: self.data }
		#print(json.dumps( obj))

		output = afnetwork.sendServer( json.dumps( obj), True, verbose)

		if output[0] == True:
			return output[1]
		else:
			return None

	def getJobList( self, verbose = False):
		self.action = 'get'
		self.data['type'] = 'jobs'
		return self._sendRequest(verbose)

	def deleteJob(self, jobName, verbose = False):
		self.action = 'action'
		self.data['type'] = 'jobs'
		self.data['mask'] = jobName
		self.data['operation'] = {'type':'delete'}
		return self._sendRequest(verbose)

	def getJobInfo(self, jobId, verbose = False):
		#self.getjobinfo(jobId)
		return self._sendRequest(verbose)

	def renderSetNimby( self, text):
		self.action = 'action'
		self.data['type'] = 'renders'
		self.data['mask'] = afenv.VARS['HOSTNAME']
		self.data['params'] = {'nimby':True}
		self._sendRequest()

	def renderSetNIMBY( self, text):
		self.action = 'action'
		self.data['type'] = 'renders'
		self.data['mask'] = afenv.VARS['HOSTNAME']
		self.data['params'] = {'NIMBY':True}
		self._sendRequest()

	def renderSetFree( self, text):
		self.action = 'action'
		self.data['type'] = 'renders'
		self.data['mask'] = afenv.VARS['HOSTNAME']
		self.data['params'] = {'nimby':False}
		self._sendRequest()

	def renderEjectTasks( self, text):
		self.action = 'action'
		self.data['type'] = 'renders'
		self.data['mask'] = afenv.VARS['HOSTNAME']
		self.data['operation'] = {'type':'eject_tasks'}
		self._sendRequest()

	def renderEjectNotMyTasks( self, text):
		self.action = 'action'
		self.data['type'] = 'renders'
		self.data['mask'] = afenv.VARS['HOSTNAME']
		self.data['operation'] = {'type':'eject_tasks_keep_my'}
		self._sendRequest()

	def renderExit( self, text):
		self.action = 'action'
		self.data['type'] = 'renders'
		self.data['mask'] = afenv.VARS['HOSTNAME']
		self.data['operation'] = {'type':'exit'}
		self._sendRequest()

	def talkExit( self, text):
		self.action = 'action'
		self.data['type'] = 'talks'
		self.data['mask'] = afenv.VARS['USERNAME'] + '@' + afenv.VARS['HOSTNAME']
		self.data['operation'] = {'type':'exit'}
		#self.talkexit( afenv.VARS['USERNAME'] + '@' + afenv.VARS['HOSTNAME'] + ':.*', text)
		self._sendRequest()

	def monitorExit( self, text):
		self.action = 'action'
		self.data['type'] = 'monitors'
		self.data['mask'] = afenv.VARS['USERNAME'] + '@' + afenv.VARS['HOSTNAME']
		self.data['operation'] = {'type':'exit'}
		#self.monitorexit( afenv.VARS['USERNAME'] + '@' + afenv.VARS['HOSTNAME'] + ':.*', text)
		self._sendRequest()

	def renderGetList( self, mask = None):
		self.action = 'get'
		self.data['type'] = 'renders'
		if mask is not None:
			self.data['mask'] = mask
		return self._sendRequest()

	def renderGetLocal( self): return self.renderGetList( afenv.VARS['HOSTNAME'])
