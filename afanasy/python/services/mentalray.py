# -*- coding: utf-8 -*-
from services import service

parser='mentalray'

str_hosts = '@AF_HOSTS@'
str_hostsprefix = '-hosts '
str_hostseparator = ' '

class mentalray ( service.service ) :
   'Mental Ray Standalone'
   # override base service class method 
   def __init__( self, taskInfo ):
    print 'mentalray.init'
    return service.service.__init__( self, taskInfo )
     
   def applycmdhosts ( self, command ):
      hosts = str_hostseparator.join ( self.hosts )
      command = command.replace( self.str_hosts, str_hostsprefix + hosts + ' -- ')
      print ( 'Mentalray hosts list "%s" applied:' % str ( hosts) )
      print ( command )
      return command
