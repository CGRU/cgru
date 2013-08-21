# -*- coding: utf-8 -*-

import cgrupathmap, cgruconfig
import os, sys, re

str_capacity = '@AF_CAPACITY@'
str_hosts = '@AF_HOSTS@'
str_hostsprefix = '-H '
str_hostseparator = ','

class service:
	"This is base service class."
	def __init__( self, taskInfo):
		self.wdir = taskInfo['wdir']
		self.command = taskInfo['command']
		self.capacity = taskInfo['capacity']
		self.hosts = taskInfo['hosts']

		self.files = taskInfo.get('files', '')
		
		self.taskInfo = taskInfo
		
		self.pm = cgrupathmap.PathMap()

		self.str_capacity = str_capacity
		self.str_hosts = str_hosts
		self.str_hostsprefix = str_hostsprefix
		self.str_hostseparator = str_hostseparator

		
	def getWDir( self):
		return self.pm.toClient( self.wdir)


	def getCommand( self):
		command = self.pm.toClient( self.command)
		# Apply capacity:
		if self.capacity > 0: command = self.applyCmdCapacity( command)
		# Apply hosts (multihosts tasks):
		if len( self.hosts): command = self.applyCmdHosts( command)
		return command


	def getFiles( self):
		if len(self.files) < 1: return self.files
		return self.pm.toClient( self.files)


	def applyCmdCapacity( self, command):
		command = command.replace( self.str_capacity, str( self.capacity))
		print('Capacity coefficient %s applied:' % str( self.capacity))
		print(command)
		return command


	def applyCmdHosts( self, command):
		hosts = str_hostsprefix
		firsthost = True
		for host in self.hosts:
			if firsthost:
				firsthost = False
			else:
				hosts += self.str_hostseparator
			hosts += host
		command = command.replace( self.str_hosts, hosts)
		print('Hosts list "%s" applied:' % str( hosts))
		print(command)
		return command


	def doPost( self):
		def check_flag(byte, flag_name):
			return True
			flags = {
					'numeric': 0x01,
					'thumbnails': 0x64
					}
			if flags[flag_name]:
				mask = flags.get(flag_name)
				return byte & mask
			else:
				return 0
		post_cmds = []
#		if len( self.files) and check_flag( self.taskInfo.get('block_flags', 0), 'thumbnails'):
#			post_cmds.extend( self.generateThumbnail())
		post_cmds.extend(['ls -la > ' + self.taskInfo['store_dir'] + '/afile'])
		return post_cmds


	def generateThumbnail( self):
		files_list = self.files.decode('utf-8').split(';')
		cmds = []

		if not os.path.isdir( self.taskInfo['store_dir']):
			return cmds

		for image in files_list:
			if len( image) < 1: continue

			basename, ext = os.path.splitext( os.path.basename( image))
			if not ext.lower() in cgruconfig.VARS['af_thumbnail_extensions']: continue

			thumbnail = image + '.jpg'
			thumbnail = os.path.join( self.taskInfo['store_dir'], thumbnail)

			self.taskInfo['image'] = image
			self.taskInfo['thumbnail'] = thumbnail

			cmds.append( cgruconfig.VARS['af_thumbnail_cmd'] % (self.taskInfo))

		return cmds


	def checkfiles( self, sizemin, sizemax):
		print('Checking for "'+self.files+'" '+str(sizemin)+'-'+str(sizemax))
		if self.files == '':
			print('Error: service::checkfiles: Files not set!')
			return False
		return True

