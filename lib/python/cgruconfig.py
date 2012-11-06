#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import socket
import stat
import sys
import time
import json

import cgruutils

VARS = dict()

def checkConfigFile( path):
	status = True
	if not os.path.isfile( path):
		try:
			cfile = open( path, 'w')
		except:
			print(str(sys.exc_info()[1]))
			status = False
		if status:
			cfile.write('{"cgru_config":{\n')
			cfile.write('"":"Created by CGRU Keeper at '+time.ctime()+'",\n')
			cfile.write('"":""\n')
			cfile.write('}}\n')
			cfile.close()
	if status:
		try:
			os.chmod( path, stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)
		except:
			print(str(sys.exc_info()[1]))
	return status


class Config:
	def __init__( self, variables = VARS, configfiles = None, Verbose = False):
		self.verbose = Verbose
		self.Vars = variables

		if configfiles is None:
			self.Vars['hostname'] = socket.gethostname().lower()

			cgrulocation =  os.getenv('CGRU_LOCATION')
			if cgrulocation is None or cgrulocation == '': return

			# Definitions which always must preset:
			self.Vars['CGRU_LOCATION'] = cgrulocation
			self.Vars['CGRU_VERSION'] = os.getenv('CGRU_VERSION','')
			self.Vars['CGRU_PYTHONEXE'] = os.getenv('CGRU_PYTHONEXE','python')
			self.Vars['CGRU_UPDATE_CMD'] = os.getenv('CGRU_UPDATE_CMD')

			self.Vars['company'] = 'CGRU'
			self.Vars['menu_path'] = None
			self.Vars['tray_icon'] = None
			self.Vars['icons_path'] = None
			if sys.platform.find('win') == 0:
				self.Vars['editor'] = 'notepad "%s"'
			else:
				self.Vars['editor'] = 'xterm -e vi "%s"'

			self.Vars['config_file'] = os.path.join( cgrulocation, 'config.json')
			home = os.getenv('HOME', os.getenv('HOMEPATH'))
			self.Vars['HOME'] = home
			self.Vars['HOME_CGRU'] = os.path.join( home, '.cgru')
			self.Vars['HOME_AFANASY'] = os.path.join( home, '.afanasy')
			self.Vars['config_file_home'] = os.path.join( self.Vars['HOME_CGRU'], 'config.json')
			self.Vars['config_afanasy'] = os.path.join( self.Vars['HOME_AFANASY'], 'config.json')
			if sys.platform.find('win') == 0 or os.geteuid() != 0:
				cgruutils.createFolder( self.Vars['HOME_CGRU']	 )
				cgruutils.createFolder( self.Vars['HOME_AFANASY'] )
				# Create cgru home config file if not preset
				checkConfigFile( self.Vars['config_file_home'])
				# Create afanasy home config file if not preset
				checkConfigFile( self.Vars['config_afanasy'])

			configfiles = []
			configfiles.append( os.path.join( cgrulocation, 'config_default.json'))
			configfiles.append( self.Vars['config_file'])
			configfiles.append( self.Vars['config_file_home'])
			configfiles.append( self.Vars['config_afanasy'])

		for filename in configfiles:
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
				print( filename)
				print( str(sys.exc_info()[1]))

			if False == success:
				continue

			for key in obj:
				if len(key):
					self.Vars[key] = obj[key];

Config()

def writeVars( variables, configfile = VARS['config_file_home']):
	file = open( configfile,'r')
	lines = file.readlines()
	file.close()
	for var in variables:
		tofind = '"'+var+'":'
		toinsert = '	"'+var+'":'+json.dumps(VARS[var])+',\n'
		founded = False
		num = -1
		for line in lines:
			num += 1
			if line.find( tofind) == -1:
				continue
			founded = True
			lines[num] = toinsert
			break
		if not founded:
			num = 1
			for line in lines:
				num += 1
				if line.find('"":""') != -1:
					continue
				lines.insert( num, toinsert)
				break
	file = open( configfile,'w')
	for line in lines: file.write( line)
	file.close()
