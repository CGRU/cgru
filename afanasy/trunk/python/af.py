#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

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
      pyaf.Task.__init__( self)
      if taskname != '': self.setName( taskname) 
      self.env = afenv.Env()
      if self.env.valid == False: print('ERROR: Invalid environment, may be some problems.')
      self.pm = PathMap( self.env.Vars['afroot'])

   def setCommand( self, cmd, TransferToServer = True):
      if TransferToServer: cmd = self.pm.toServer( cmd)
      pyaf.Task.setCommand( self, cmd)
   def setFiles( self, cmd, TransferToServer = True):
      if TransferToServer: cmd = self.pm.toServer( cmd)
      pyaf.Task.setFiles( self, cmd)

class Block(pyaf.Block):
   def __init__( self, blockname = 'block', service = 'generic'):
      self.env = afenv.Env()
      if self.env.valid == False: print('ERROR: Invalid environment, may be some problems.')
      self.pm = PathMap( self.env.Vars['afroot'])
      pyaf.Block.__init__( self)
      parser = ''
      if not CheckClass( self.env.Vars['afroot'], service, 'services'):
         print('Error: Unknown service "%s", setting to "generic"' % service)
         service = 'generic'
      else:
         __import__("services", globals(), locals(), [service])
         parser = eval(('services.%s.parser') % service)
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

   def setNumeric( self, start = 1, end = 10, perhost = 1, increment = 1):
      pyaf.Block.setNumeric( self, start, end, perhost, increment)
   def setCapacity( self, capacity):
      if capacity >= 0: pyaf.Block.setCapacity( self, capacity)
   def setVariableCapacity( self, capmin, capmax):
      if capmin >= 0 or capmax >= 0: pyaf.Block.setVariableCapacity( self, capmin, capmax)

   def setWorkingDirectory( self, cmd, TransferToServer = True):
      if TransferToServer: cmd = self.pm.toServer( cmd)
      pyaf.Block.setWorkingDirectory( self, cmd)
   def setCommand( self, cmd, prefix = True, TransferToServer = True):
      if prefix: cmd = os.getenv('AF_CMD_PREFIX', self.env.Vars['cmdprefix']) + cmd
      if TransferToServer: cmd = self.pm.toServer( cmd)
      pyaf.Block.setCommand( self, cmd)
   def setCmdPre(  self, cmd, TransferToServer = True):
      if TransferToServer: cmd = self.pm.toServer( cmd)
      pyaf.Block.setCmdPre(  self, cmd)
   def setCmdPost( self, cmd, TransferToServer = True):
      if TransferToServer: cmd = self.pm.toServer( cmd)
      pyaf.Block.setCmdPost( self, cmd)
   def setFiles(  self, cmd, TransferToServer = True):
      if TransferToServer: cmd = self.pm.toServer( cmd)
      pyaf.Block.setFiles(  self, cmd)

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
      pyaf.Job.__init__( self)
      self.env = afenv.Env( verbose)
      if self.env.valid == False: print('ERROR: Invalid environment, may be some problems.')
      self.pm = PathMap(            self.env.Vars['afroot'], False, verbose)
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
      username = username.lower()
      pyaf.Job.setUserName( self, username)

   def setPriority( self, priority):
      if priority < 0: return
      if priority > 250:
         priority = 250
         print('Warning: priority clamped to maximum = %d' % priority)
      pyaf.Job.setPriority( self, priority)

   def setCmdPre(  self, cmd, TransferToServer = True):
      if TransferToServer: cmd = self.pm.toServer( cmd)
      pyaf.Job.setCmdPre(  self, cmd)
   def setCmdPost( self, cmd, TransferToServer = True):
      if TransferToServer: cmd = self.pm.toServer( cmd)
      pyaf.Job.setCmdPost( self, cmd)

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
      self.fillBlocks()
      if self.construct() == False: return False
      return afnetwork.sendServer( self.getData(), self.getDataLen(), self.env.Vars['servername'], int(self.env.Vars['serverport']), verbose)[0]

   def pause(      self): self.offline()
   def setPaused(  self): self.offline()
   def offLine(    self): self.offline()
   def setOffline( self): self.offline()


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
      output = afnetwork.sendServer( self.getData(), self.getDataLen(), self.env.Vars['servername'], int(self.env.Vars['serverport']), verbose)
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
      self.rendersetnimby( self.env.Vars['hostname'] + '.*', text)
      self._sendRequest()

   def renderSetNIMBY( self, text):
      self.rendersetNIMBY( self.env.Vars['hostname'] + '.*', text)
      self._sendRequest()

   def renderSetFree( self, text):
      self.rendersetfree( self.env.Vars['hostname'] + '.*', text)
      self._sendRequest()

   def renderEjectTasks( self, text):
      self.renderejecttasks( self.env.Vars['hostname'] + '.*', text)
      self._sendRequest()

   def renderExit( self, text):
      self.renderexit( self.env.Vars['hostname'] + '.*', text)
      self._sendRequest()

   def talkExit( self, text):
      self.talkexit( self.env.Vars['username'] + '@' + self.env.Vars['hostname'] + '.*', text)
      self._sendRequest()

   def monitorExit( self, text):
      self.monitorexit( self.env.Vars['username'] + '@' + self.env.Vars['hostname'] + '.*', text)
      self._sendRequest()

   def renderGetLocal( self): return self.renderGetList( self.env.Vars['hostname'] + '.*')
   def renderGetList( self, mask = '.*'):
      if not self.renderlistget( mask): return
      if self._sendRequest():
         return self.renderlistdecode( self.requestOutput)
