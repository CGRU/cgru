#!/usr/bin/python2.5
# -*- coding: utf-8 -*-

import os
import socket
import sys

def sendServer( data, datalen, host, port, verbose = False):
   s = None
   for res in socket.getaddrinfo( host, port, socket.AF_UNSPEC, socket.SOCK_STREAM):
#      print res
      af, socktype, proto, canonname, sa = res
      if verbose: print 'Trying to connect to "%s"' % str(sa[0])
      try:
         s = socket.socket(af, socktype, proto)
      except socket.error, msg:
         print str(sys.exc_info()[1])
         s = None
         continue
      try:
         s.connect(sa)
      except socket.error, msg:
         print str(sys.exc_info()[1])
         s.close()
         s = None
         continue
      break

   if s is None:
      print 'Could not open socket.'
      return False

   if verbose: print 'afnetwork.sendServer: send %d bytes' % datalen
   s.sendall( data)
   s.close()
   return True
