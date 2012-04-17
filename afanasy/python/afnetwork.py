#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import socket
import sys

import afenv

AF_SENDERID=0

def genHeader( data_size):
	data = '[ * AFANASY * ]'
	data += ' ' + str(int(afenv.VARS["magic_number"]))
	data += ' ' + str(AF_SENDERID)
	data += ' ' + str(data_size)
	data += ' JSON'
	return bytearray( data, 'utf-8')

def sendServer( data, receive = True, verbose = False):

	size = len(data)
	header = genHeader(size)
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

	struct = None

	try:
		struct = json.loads(data)
	except:
		print( data)
		print( str(sys.exc_info()[1]))
		struct = None

	return True, struct
