#!/usr/bin/env python

import os
import sys

path = os.path.abspath(sys.argv[0])
sys.path.append( path)

from afpathmap import PathMap

afroot = os.path.dirname( path)
afroot = os.path.dirname( afroot)

if len(sys.argv) <= 1:
   print('\nUsage: To check command pathes transfer launch:')
   print( os.path.basename(sys.argv[0]) + ' [command|file]\n')

if len(sys.argv) > 1:
   path = sys.argv[1]
   if os.path.isfile( path):
      pm = PathMap( afroot, True, True)
      path_server = path + '_server'
      path_client = path + '_client'
      print('To server filename = "%s"' % path_server)
      pm.toServerFile( path, path_server, Verbose = False)
      print('To client filename = "%s"' % path_client)
      pm.toClientFile( path_server, path_client, Verbose = False)
   else:
      path = ''
      for arg in range( 1, len( sys.argv)):
         if arg != 1: path += ' '
         path += sys.argv[arg]
      pm = PathMap( afroot, False, True)
      print('                  1         2         3         4         5         6         7')
      print('        012345678901234567890123456789012345678901234567890123456789012345678901234567890')
      print('Origin: ' + path)
      path = pm.toServer( path, Verbose = False)
      print('Server: ' + path)
      path = pm.toClient( path, Verbose = False)
      print('Client: ' + path)
