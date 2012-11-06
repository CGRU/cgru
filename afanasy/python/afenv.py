#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
import socket
import json

VARS = dict()

class Initialize:
	def __init__( self, Verbose = False):
		self.verbose = Verbose
		self.valid = False

		cgru = os.getenv('CGRU_LOCATION')

		afroot = os.getenv('AF_ROOT')
		if afroot is None:
			print('Error: AF_ROOT is not defined.')
			return

		home = os.getenv('HOME', os.getenv('HOMEPATH'))
		if home == None: home = 'None'

		username = os.getenv('AF_USERNAME', os.getenv('USER', os.getenv('USERNAME')))
		if username == None: username = 'None'
		# cut DOMAIN from username:
		dpos = username.rfind('/')
		if dpos == -1: dpos = username.rfind('\\')
		if dpos != -1: username = username[dpos+1:]
		username = username.lower()

		hostname = os.getenv('AF_HOSTNAME', socket.gethostname()).lower()
		if self.verbose:
			print('Afanasy root = "%s"' % afroot)
			print('Host name = "%s"' % hostname)
			print('User name = "%s"' % username)
			print('User home = "%s"' % home)

		VARS['AF_ROOT'] = afroot
		VARS['HOME'] = home
		VARS['USERNAME'] = username
		VARS['HOSTNAME'] = hostname
		VARS['HOME_AFANASY'] = os.path.join( home, '.afanasy')

		filenames = []
		if cgru:
			VARS['CGRU_LOCATION'] = cgru
			filenames.extend([cgru+'/config_default.json', cgru+'/config.json', home+'/.cgru/config.json'])
		filenames.extend([afroot+'/config_default.json', afroot+'/config.json', home+'/.afanasy/config.json'])

		for filename in filenames:
			if self.verbose:
				print('Trying to open %s' % filename)
			if not os.path.isfile( filename):
				continue

			file = open( filename, 'r')
			filedata = file.read()
			file.close()

			success = True
			try:
				obj = json.loads( filedata)['cgru_config']
			except:
				success = False
				self.valid = False
				print( filename)
				print( str(sys.exc_info()[1]))

			if False == success:
				continue

			for key in obj:
				if len(key):
					VARS[key] = obj[key];

		self.valid = True
		if self.verbose: print(self.Vars)

Initialize()
