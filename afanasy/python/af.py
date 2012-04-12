#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

import json

import time

import pyaf
print('Python module = "%s"' % pyaf.__file__)

import afenv
import afnetwork
import services

from afpathmap import PathMap

def CheckClass( afroot, name, folder):
	filename = name + '.py'
	path = os.path.join( afroot, 'python')
	path = os.path.join( path, folder)
	if filename in os.listdir( path): return True
	return False

class Task(pyaf.Task):
	def __init__( self, taskname = ''):
		self.env = afenv.Env()
		if self.env.valid == False: print('ERROR: Invalid environment, may be some problems.')
		self.pm = PathMap( self.env.Vars['afroot'])
		self.data = dict()

		pyaf.Task.__init__( self)
		if taskname != '': self.setName( taskname)

	def setCommand( self, command, TransferToServer = True):
		if TransferToServer: command = self.pm.toServer( command)

		pyaf.Task.setCommand( self, command)

	def setFiles( self, files, TransferToServer = True):
		if TransferToServer: files = self.pm.toServer( files)

		pyaf.Task.setFiles( self, files)

class Block(pyaf.Block):
	def __init__( self, blockname = 'block', service = 'generic'):
		self.env = afenv.Env()
		if self.env.valid == False: print('ERROR: Invalid environment, may be some problems.')
		self.pm = PathMap( self.env.Vars['afroot'])
		parser = ''
		if not CheckClass( self.env.Vars['afroot'], service, 'services'):
			print('Error: Unknown service "%s", setting to "generic"' % service)
			service = 'generic'
		else:
			__import__("services", globals(), locals(), [service])
			parser = eval(('services.%s.parser') % service)

		self.data = dict()
		self.data["name"] = blockname
		self.data["service"] = service
		self.data["parser"] = parser
		self.data["capacity"] = int( self.env.Vars['task_default_capacity'])
		self.data["working_directory"] = os.getenv('PWD', os.getcwd())

		pyaf.Block.__init__( self)

		self.setName( blockname)
		self.setService( service)
		self.setParser( parser)
		self.setWorkingDirectory( os.getenv('PWD', os.getcwd()) )
		self.setCapacity( int( self.env.Vars['task_default_capacity'] ) )
		self.tasks = []

	def setParser( self, parser, nocheck = False):
		if parser != '':
			if not nocheck:
				if not CheckClass( self.env.Vars['afroot'], parser, 'parsers'):
					if parser != 'none':
						print('Error: Unknown parser "%s", setting to "none"' % parser)
						parser = 'none'
		pyaf.Block.setParser( self, parser)

	def setNumeric( self, start = 1, end = 10, pertask = 1, increment = 1):
		if end < start:
			print('Error: Block.setNumeric: end < start (%d < %d)' % (end,start))
			end = start
		if pertask < 1:
			print('Error: Block.setNumeric: pertask < 1 (%d < 1)' % pertask)
			pertask = 1

		pyaf.Block.setNumeric( self, start, end, pertask, increment)

	def setCapacity( self, capacity):
		if capacity < 0:
			print('Error: Block.setCapacity: capacity < 0 (%d < 0)' % capacity)
			return
		self.data["capacity"] = capacity

		pyaf.Block.setCapacity( self, capacity)

	def setVariableCapacity( self, capacity_coeff_min, capacity_coeff_max):
		if capacity_coeff_min >= 0 or capacity_coeff_max >= 0:
			self.data["capacity_coeff_min"] = capacity_coeff_min
			self.data["capacity_coeff_max"] = capacity_coeff_max

			pyaf.Block.setVariableCapacity( self, capacity_coeff_min, capacity_coeff_max)

	def setWorkingDirectory( self, working_directory, TransferToServer = True):
		if TransferToServer: working_directory = self.pm.toServer( working_directory)
		self.data["working_directory"] = working_directory

		pyaf.Block.setWorkingDirectory( self, working_directory)

	def setCommand( self, command, prefix = True, TransferToServer = True):
		if prefix: command = os.getenv('AF_CMD_PREFIX', self.env.Vars['cmdprefix']) + command
		if TransferToServer: command = self.pm.toServer( command)
		self.data["command"] = command

		pyaf.Block.setCommand( self, command)

	def setCmdPre(  self, command_pre, TransferToServer = True):
		if TransferToServer: command_pre = self.pm.toServer( command_pre)
		self.data["command_pre"] = command_pre

		pyaf.Block.setCmdPre(  self, command_pre)

	def setCmdPost( self, command_post, TransferToServer = True):
		if TransferToServer: command_post = self.pm.toServer( command_post)
		self.data["command_post"] = command_post

		pyaf.Block.setCmdPost( self, command_post)

	def setFiles(  self, files, TransferToServer = True):
		if TransferToServer: files = self.pm.toServer( files)
		self.data["files"] = files

		pyaf.Block.setFiles(  self, files)

	def fillTasks( self):
		self.clearTasksList()
		t = 0
		for task in self.tasks:
			if isinstance( task, Task):
				self.appendTask( task)
			else:
				print('Warning: Skipping element[%d] of list "tasks" which is not an instance of "Task") class:' % t)
				print(repr(task))
			t += 1

class Job(pyaf.Job):
	def __init__( self, jobname = None, verbose = False):
		self.env = afenv.Env( verbose)
		if self.env.valid == False: print('ERROR: Invalid environment, may be some problems.')
		self.pm = PathMap( self.env.Vars['afroot'], False, verbose)

		self.data = dict()
		self.data["name"] = "noname"
		self.data["user_name"] = self.env.Vars['username']
		self.data["host_name"] = self.env.Vars['hostname']
		self.data["priority"]  = self.env.Vars['priority']
		self.data["time_creation"] = int(time.time())
		self.data["blocks"] = []

		pyaf.Job.__init__( self)
		self.setPriority(        int( self.env.Vars['priority']       ))
		self.setUserName(             self.env.Vars['username']        )
		self.setHostName(             self.env.Vars['hostname']        )
		if jobname != None: self.setName( jobname)
		platform = sys.platform
		# looking at 'darwin' at first as its name contains 'win' sting too
		if platform.find('darwin') > -1: self.setNeedOS( 'mac')
		elif platform.find('win') > -1: self.setNeedOS( 'win')
		else: self.setNeedOS( 'linux')
		self.blocks = []

	def setUserName( self, username):
		if username == None or username == '':
			username = 'none'
			print('Error: Username is empty.')
		self.data["user_name"] = username.lower()

		username = username.lower()
		pyaf.Job.setUserName( self, username)

	def setPriority( self, priority):
		if priority < 0: return
		if priority > 250:
			priority = 250
			print('Warning: priority clamped to maximum = %d' % priority)
		self.data["priority"] = priority

		pyaf.Job.setPriority( self, priority)

	def setCmdPre(  self, command, TransferToServer = True):
		if TransferToServer: command = self.pm.toServer( command)
		self.data["command_pre"] = command

		pyaf.Job.setCmdPre(  self, command)

	def setCmdPost( self, command, TransferToServer = True):
		if TransferToServer: command = self.pm.toServer( command)
		self.data["command_post"] = command

		pyaf.Job.setCmdPost( self, command)

	def fillBlocks( self):
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

		for block in self.blocks:
			self.data["blocks"].append( block.data)
		obj = {"job": self.data }
		print(json.dumps( obj))

		self.fillBlocks()
		if self.construct() == False: return False
		return afnetwork.sendServer( self.getData(), self.getDataLen(), self.env.Vars['servername'], int(self.env.Vars['serverport']), False, verbose)[0]

	def pause(      self): self.offline()
	def setPaused(  self): self.offline()
	def setOffline( self): self.offline()
	def pause(      self): self.data["pause"] = true
	def setPaused(  self): self.data["pause"] = true
	def setOffline( self): self.data["pause"] = true
	def offline(    self): self.data["pause"] = true


class Cmd( pyaf.Cmd):
   def __init__( self ):
      self.env = afenv.Env()
      if self.env.valid == False:
         print('ERROR: Invalid environment, may be some problems.')
      self.pm = PathMap( self.env.Vars['afroot'])
      pyaf.Cmd.__init__( self, self.env.Vars['servername'], self.env.Vars['serverport'])
      pyaf.Cmd.setUserName( self, self.env.Vars['username'])
      pyaf.Cmd.setHostName( self, self.env.Vars['hostname'])
      self.requestOutput = None
   
   def _sendRequest(self, verbose = False):
      if self.getDataLen() < 1:
         print('ERROR: Request message is not set.')
         return False
      output = afnetwork.sendServer( self.getData(), self.getDataLen(), self.env.Vars['servername'], int(self.env.Vars['serverport']), True, verbose)
      if output[0] == True:
         self.requestOutput = output[1]
         return True
      else:
         self.requestOutput = None
         return False

   def getJobList( self, verbose = False):
      self.getjoblist(0)
      if self._sendRequest(verbose):
         return self.decodejoblist(self.requestOutput)
      else:
         return False

   def deleteJob(self, jobName, verbose = False):
      self.deletejob(jobName)
      if self._sendRequest(verbose):
         return True
      else:
         return False

   def getJobInfo(self, jobId, verbose = False):
      self.getjobinfo(jobId)
      if self._sendRequest(verbose):
         return self.decodejobinfo(self.requestOutput)
      else:
         return False

   def renderSetNimby( self, text):
      self.rendersetnimby( self.env.Vars['hostname'], text)
      self._sendRequest()

   def renderSetNIMBY( self, text):
      self.rendersetNIMBY( self.env.Vars['hostname'], text)
      self._sendRequest()

   def renderSetFree( self, text):
      self.rendersetfree( self.env.Vars['hostname'], text)
      self._sendRequest()

   def renderEjectTasks( self, text):
      self.renderejecttasks( self.env.Vars['hostname'], text)
      self._sendRequest()

   def renderEjectNotMyTasks( self, text):
      self.renderejectnotmytasks( self.env.Vars['hostname'], text)
      self._sendRequest()

   def renderExit( self, text):
      self.renderexit( self.env.Vars['hostname'], text)
      self._sendRequest()

   def talkExit( self, text):
      self.talkexit( self.env.Vars['username'] + '@' + self.env.Vars['hostname'] + ':.*', text)
      self._sendRequest()

   def monitorExit( self, text):
      self.monitorexit( self.env.Vars['username'] + '@' + self.env.Vars['hostname'] + ':.*', text)
      self._sendRequest()

   def renderGetLocal( self): return self.renderGetList( self.env.Vars['hostname'])
   def renderGetList( self, mask = '.*'):
      if not self.renderlistget( mask): return
      if self._sendRequest():
         return self.renderlistdecode( self.requestOutput)
