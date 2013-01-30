# -*- coding: utf-8 -*-
import socket
from services import service

parser='vray'

str_hosts = '@AF_HOSTS@'
str_hostsprefix = '-renderhost='
str_hostseparator = ';'

class vray(service.service):
   'VRay Standalone'
   # override base service class method 
   def __init__( self, taskInfo ):
    print 'vray.init'
    return service.service.__init__( self, taskInfo )
     
   def applycmdhosts ( self, command ):
      hosts_ip = []
      for host_name in self.hosts :
        hosts_ip.append ( socket.gethostbyname ( host_name ) )  
      hosts = str_hostseparator.join ( hosts_ip )
      command = command.replace( self.str_hosts, str_hostsprefix + '"' + hosts + '"')
      print ( 'VRay hosts list "%s" applied:' % str ( hosts) )
      print ( command )
      return command
