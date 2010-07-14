#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

import pyaf

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
      pyaf.Task.__init__( self)
      if taskname != '': self.setName( taskname) 
      self.env = afenv.Env()
      if self.env.valid == False: print 'ERROR: Invalid environment, may be some problems.'
      self.pm = PathMap( self.env.Vars['afroot'])

   def setCommand( self, cmd, TransferToServer = True):
      if TransferToServer: cmd = self.pm.toServer( str(cmd))
      pyaf.Task.setCommand( self, str(cmd))
   def setFiles( self, cmd, TransferToServer = True):
      if TransferToServer: cmd = self.pm.toServer( str(cmd))
      pyaf.Task.setFiles( self, str(cmd))

class Block(pyaf.Block):
   def __init__( self, blockname = 'block', blocktype = 'generic'):
      self.env = afenv.Env()
      if self.env.valid == False: print 'ERROR: Invalid environment, may be some problems.'
      self.pm = PathMap( self.env.Vars['afroot'])
      pyaf.Block.__init__( self)
      parsertype = 'none'
      if not CheckClass( self.env.Vars['afroot'], blocktype, 'services'):
         print 'Error: Unknown service "%s", setting to "generic"' % blocktype
         blocktype = 'generic'
      else:
         __import__("services", globals(), locals(), [blocktype])
         parsertype = eval(('services.%s.parser') % blocktype)
      self.setName( blockname)
      self.setTasksType( blocktype)
      self.setParserType( parsertype)
      self.setWorkingDirectory( os.getenv('PWD', os.getcwd()) )
      self.setCapacity( int( self.env.Vars['task_default_capacity'] ) )
      self.tasks = []

   def setParserType( self, parsertype, nocheck = False):
      if not nocheck:
         if not CheckClass( self.env.Vars['afroot'], parsertype, 'parsers'):
            if parsertype != 'none':
               print 'Error: Unknown parser "%s", setting to "none"' % parsertype
               parsertype = 'none'
      pyaf.Block.setParserType( self, parsertype)

   def setNumeric( self, start = 1, end = 10, perhost = 1, increment = 1):
      pyaf.Block.setNumeric( self, start, end, perhost, increment)
   def setCapacity( self, capacity):
      if capacity >= 0: pyaf.Block.setCapacity( self, capacity)
   def setVariableCapacity( self, capmin, capmax):
      if capmin >= 0 or capmax >= 0: pyaf.Block.setVariableCapacity( self, capmin, capmax)

   def setWorkingDirectory( self, cmd, TransferToServer = True):
      if TransferToServer: cmd = self.pm.toServer( str(cmd))
      pyaf.Block.setWorkingDirectory( self, str(cmd))
   def setCommand( self, cmd, prefix = True, TransferToServer = True):
      if prefix: cmd = os.getenv('AF_CMD_PREFIX', self.env.Vars['cmdprefix']) + str(cmd)
      if TransferToServer: cmd = self.pm.toServer( str(cmd))
      pyaf.Block.setCommand( self, str(cmd))
   def setCmdPre(  self, cmd, TransferToServer = True):
      if TransferToServer: cmd = self.pm.toServer( str(cmd))
      pyaf.Block.setCmdPre(  self, str(cmd))
   def setCmdPost( self, cmd, TransferToServer = True):
      if TransferToServer: cmd = self.pm.toServer( str(cmd))
      pyaf.Block.setCmdPost( self, str(cmd))
   def setFiles(  self, cmd, TransferToServer = True):
      if TransferToServer: cmd = self.pm.toServer( str(cmd))
      pyaf.Block.setFiles(  self, str(cmd))

###### DEPRECATED: ######
   def setCommandView(  self, cmd, TransferToServer = True):
      print 'Warning: Method "Block.setCommandView" is deprecated, use "Block.setFiles" instead.'
      self.setFiles( cmd , TransferToServer)
   def addTask( self, taskname = ''):
      print 'Warning: Method "Job.addTask" is deprecated, use "Block.tasks" list instead.'
      if taskname == '': taskname = 'task #' + str(len(self.tasks))
      task = Task( taskname)
      self.tasks.append( task)
      return task
#########################

   def fillTasks( self):
      self.clearTasksList()
      t = 0
      for task in self.tasks:
         if isinstance( task, Task):
            self.appendTask( task)
         else:
            print 'Warning: Skipping element[%d] of list "tasks" which is not an instance of "Task" class:' % t
            print str(task)
         t += 1

class Job(pyaf.Job):
   def __init__( self, jobname = None, verbose = False):
      pyaf.Job.__init__( self)
      self.env = afenv.Env( verbose)
      if self.env.valid == False: print 'ERROR: Invalid environment, may be some problems.'
      self.pm = PathMap( self.env.Vars['afroot'])
      self.setPriority(  int( self.env.Vars['priority'] ))
      self.setMaxHosts(  int( self.env.Vars['maxhosts'] ))
      self.setUserName(       self.env.Vars['username']  )
      self.setHostName(       self.env.Vars['hostname']  )
      self.setHostsMask(      self.env.Vars['hostsmask'] )
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
         print 'Error: Username is empty.'
      username = username.lower()
      pyaf.Job.setUserName( self, username)

   def setPriority( self, priority):
      if priority < 0: return
      if priority > 250:
         priority = 250
         print 'Warning: priority clamped to maximum = %d' % priority
      pyaf.Job.setPriority( self, priority)

   def setCmdPre(  self, cmd, TransferToServer = True):
      if TransferToServer: cmd = self.pm.toServer( str(cmd))
      pyaf.Job.setCmdPre(  self, str(cmd))
   def setCmdPost( self, cmd, TransferToServer = True):
      if TransferToServer: cmd = self.pm.toServer( str(cmd))
      pyaf.Job.setCmdPost( self, str(cmd))

   def fillBlocks( self):
      self.clearBlocksList()
      b = 0
      for block in self.blocks:
         if isinstance( block, Block):
            block.fillTasks()
            self.appendBlock( block)
         else:
            print 'Warning: Skipping element[%d] of list "blocks" which is not an instance of "Block" class:' % b
            print str(block)
         b += 1

   def output( self, full = False):
      self.fillBlocks()
      if full: print 'Job information:'
      else:    print 'Job: ',
      pyaf.Job.output( self, full)

   def send( self, verbose = False):
      if len( self.blocks) == 0:
         print 'Error: Job has no blocks'
         return False
      self.fillBlocks()
      if self.construct() == False: return False
      return afnetwork.sendServer( self.getData(), self.getDataLen(), self.env.Vars['servername'], int(self.env.Vars['serverport']), verbose)

   def addBlock( self, blockname = 'block', blocktype = 'generic'):
      print 'Warning: Method "Job::addBlock" is depricated, use job "blocks" list instead.'
      block = Block( blockname, blocktype)
      self.blocks.append( block)
      return block
