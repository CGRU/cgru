#!/usr/bin/env python

import os
import sys

from cgrupathmap import PathMap

if len(sys.argv) <= 1:
	print('\nUsage: To check command pathes transfer launch:')
	print( os.path.basename(sys.argv[0]) + ' [unix] [command|file] [stings...]\n')
	sys.exit(1)

ArgNum = 1
UnixSeparators = False
if sys.argv[ArgNum] == 'unix':
	UnixSeparators = True
	ArgNum = 2

path = sys.argv[ArgNum]

if os.path.isfile( path):
	strings = []
	for i in range( ArgNum+1, len(sys.argv)):
		strings.append( sys.argv[i])
	if len(strings):
		print('Search for srings:')
		print(strings)

	pm = PathMap( UnixSeparators, Verbose = True)
	path_server = path + '_server'
	path_client = path + '_client'
	print('To server filename = "%s"' % path_server)
	pm.toServerFile( path, path_server, strings, Verbose = False)
	print('To client filename = "%s"' % path_client)
	pm.toClientFile( path_server, path_client, strings, Verbose = False)
else:
	path = ''
	for arg in range( ArgNum, len( sys.argv)):
		if arg != ArgNum: path += ' '
		path += sys.argv[arg]
	pm = PathMap( UnixSeparators, Verbose = True)
	print('                  1         2         3         4         5         6         7')
	print('        012345678901234567890123456789012345678901234567890123456789012345678901234567890')
	print('Origin: ' + path)
	path = pm.toServer( path, Verbose = False)
	print('Server: ' + path)
	path = pm.toClient( path, Verbose = False)
	print('Client: ' + path)
