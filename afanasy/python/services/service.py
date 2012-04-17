# -*- coding: utf-8 -*-

import afpathmap

str_capacity = '@AF_CAPACITY@'
str_hosts = '@AF_HOSTS@'
str_hostsprefix = '-H '
str_hostseparator = ','

class service:
   "This is base service class."
   def __init__( self, wdir, command, capacity, hosts, files = ''):
      self.wdir = wdir
      self.command = command
      self.capacity = capacity
      self.hosts = hosts
      self.files = files

      self.pm = afpathmap.PathMap()

      self.str_capacity = str_capacity
      self.str_hosts = str_hosts
      self.str_hostsprefix = str_hostsprefix
      self.str_hostseparator = str_hostseparator

   def getWDir( self):
      return self.pm.toClient( self.wdir)

   def getCommand( self):
      command = self.pm.toClient( self.command)
      # Apply capacity:
      if self.capacity > 0: command = self.applycmdcapacity( command)
      # Apply hosts (multihosts tasks):
      if len( self.hosts): command = self.applycmdhosts( command)
      return command

   def getFiles( self):
      if len(self.files) < 1: return self.files
      return self.pm.toClient( self.files)

   def applycmdcapacity( self, command):
      command = command.replace( self.str_capacity, str( self.capacity))
      print('Capacity coefficient %s applied:' % str( self.capacity))
      print(command)
      return command

   def applycmdhosts( self, command):
      hosts = str_hostsprefix
      firsthost = True
      for host in self.hosts:
         if firsthost:
            firsthost = False
         else:
            hosts += self.str_hostseparator
         hosts += host
      command = command.replace( self.str_hosts, hosts)
      print('Hosts list "%s" applied:' % str( hosts))
      print(command)
      return command

   def checkfiles( self, sizemin, sizemax):
      print('Checking for "'+self.files+'" '+str(sizemin)+'-'+str(sizemax))
      if self.files == '':
         print('Error: service::checkfiles: Files not set!')
         return False
      return True
