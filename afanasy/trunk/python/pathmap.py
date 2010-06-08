#!/usr/bin/env python

import os
import sys

path = os.path.abspath(sys.argv[0])
sys.path.append( path)

import afpathmap
from afpathmap import PathMap

afroot = os.path.dirname( path)
afroot = os.path.dirname( afroot)
pm = PathMap( afroot, len(sys.argv) <= 1)

if len(sys.argv) <= 1:
   print '\nUsage: To check command pathes transfer launch:'
   print os.path.basename(sys.argv[0]) + ' [command|file]'

if len(sys.argv) > 1:
   path = sys.argv[1]
   if os.path.isfile( path):
      path_server = path + '_server'
      path_client = path + '_client'
      print 'To server filename = "%s"' % path_server
      pm.toServerFile( path, path_server, Verbose = False)
      print 'To client filename = "%s"' % path_client
      pm.toClientFile( path_server, path_client, Verbose = False)
   else:
      print 'Origin: ' + path
      path = pm.toServer( path, Verbose = False)
      print 'Server: ' + path
      path = pm.toClient( path, Verbose = False)
      print 'Client: ' + path
