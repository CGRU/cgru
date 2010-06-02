# -*- coding: utf-8 -*-

import afpathmap

str_capacity = '@AF_CAPACITY@'
str_hosts = '@AF_HOSTS@'
str_hostsprefix = '-H '
str_hostseparator = ','

class service:
   "This is base class, not to be instanced"
   def __init__( self, afroot, wdir, command, capacity, hosts, files = ''):
      self.wdir = wdir
      self.command = command
      self.capacity = capacity
      self.hosts = hosts
      self.files = files

      self.pm = afpathmap.PathMap( afroot)

      self.str_capacity = str_capacity
      self.str_hosts = str_hosts
      self.str_hostsprefix = str_hostsprefix
      self.str_hostseparator = str_hostseparator

      print 'Service:'
      print self.wdir
      print self.command

   def getWDir( self):
      return self.pm.toClient( self.wdir)

   def getCommand( self):
      return self.pm.toClient( self.command)

   def applycmdcapacity( self, capacity):
      return self.command.replace( self.str_capacity, str( capacity))

   def applycmdhosts( self, hostslist):
      hosts = str_hostsprefix
      firsthost = True
      for host in hostslist:
         if firsthost:
            firsthost = False
         else:
            hosts += self.str_hostseparator
         hosts += host
      return self.command.replace( self.str_hosts, hosts)

   def checkfiles( self, sizemin, sizemax):
      print 'Checking for "'+self.files+'" '+str(sizemin)+'-'+str(sizemax)
      if self.files == '':
         print 'Error: service::checkfiles: Files not set!'
         return False
      return True
