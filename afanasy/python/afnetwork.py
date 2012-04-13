#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import socket
import sys

import afenv

def sendServer( data, receive = True, verbose = False):

	size = len(data)
	header = bytearray((0,0,0,44, 0,0,0,1, 0,0,0,0, 0,0,0,219, 0,0,int(size/256),size-256*int(size/256)))
	data = header + bytearray( data, 'utf-8')
	datalen = len(data)
	#return True, None

	host = afenv.VARS['servername']
	port = int(afenv.VARS['serverport'])

	s = None
	err_msg = ''
	reslist = []

	try:
		reslist = socket.getaddrinfo( host, port, socket.AF_UNSPEC, socket.SOCK_STREAM)
	except:
		print('Can`t solve "%s":' % host + str(sys.exc_info()[1]))

	for res in reslist:
		af, socktype, proto, canonname, sa = res
		if verbose: print('Trying to connect to "%s"' % str(sa[0]))
		try:
			s = socket.socket(af, socktype, proto)
		except:
			if err_msg != '': err_msg += '\n'
			err_msg += str(sa[0]) + ' : ' + str(sys.exc_info()[1])
			s = None
			continue
		try:
			s.connect(sa)
		except:
			if err_msg != '': err_msg += '\n'
			err_msg += str(sa[0]) + ' : ' + str(sys.exc_info()[1])
			s.close()
			s = None
			continue
		break

	if s is None:
		print('Could not open socket.')
		print( err_msg)
		return False, None

	if verbose: print('afnetwork.sendServer: send %d bytes' % datalen)
	s.sendall( data)
   
	if not receive:
		return True, None

	data = b''
	while True:
		buffer = s.recv(4096)
		if not buffer:
			break
		data += buffer
	s.close()

	if len(data) < 22:
		return True, None
	#print(data[20:-1])

	try:
		data = json.loads(data[20:-1])
	except:
		print( str(sys.exc_info()[1]))
		data = None

	return True, data
