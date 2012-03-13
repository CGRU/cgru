#!/usr/bin/python2.5
# -*- coding: utf-8 -*-

import os
import socket

def sendServer( data, datalen, host, port, verbose = False):
   s = None
   for res in socket.getaddrinfo( host, port, socket.AF_INET, socket.SOCK_STREAM):
      af, socktype, proto, canonname, sa = res
      try:
         s = socket.socket(af, socktype, proto)
      except socket.error, msg:
         s = None
         continue
      try:
         s.connect(sa)
      except socket.error, msg:
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
