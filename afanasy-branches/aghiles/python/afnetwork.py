#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import socket
import sys

def sendServer( data, datalen, host, port, verbose = False):
   s = None
   for res in socket.getaddrinfo( host, port, socket.AF_UNSPEC, socket.SOCK_STREAM):
#      print(res)
      af, socktype, proto, canonname, sa = res
      if verbose: print('Trying to connect to "%s"' % str(sa[0]))
      try:
         s = socket.socket(af, socktype, proto)
      except:
         print(str(sys.exc_info()[1]))
         print(str(sa[0]))
         s = None
         continue
      try:
         s.connect(sa)
      except:
         print(str(sys.exc_info()[1]))
         print(str(sa[0]))
         s.close()
         s = None
         continue
      break

   if s is None:
      print('Could not open socket.')
      return False, None

   if verbose: print('afnetwork.sendServer: send %d bytes' % datalen)
   s.sendall( data)
   data = b''
   while True:
      buffer = s.recv(4096)
      if not buffer:
         break
      data += buffer
   s.close()
   return True, data
