#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import socket
import sys

def sendServer( data, datalen, host, port, receive = True, verbose = False):
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

	return True, data
