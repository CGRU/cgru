# -*- coding: utf-8 -*-

import cgrupathmap, cgruconfig
import os, sys, re

if sys.version_info[0] == 2 and sys.version_info[1] >= 6:
	import httplib as http_client
elif sys.version[0] == '3':
	import http.client as http_client


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
		if self.capacity > 0: command = self.applycmdcapacity( command)
		# Apply hosts (multihosts tasks):
		if len( self.hosts): command = self.applycmdhosts( command)
		return command

	def getFiles( self):
		if len(self.files) < 1: return self.files
		return self.pm.toClient( self.files)

	def applycmdcapacity( self, command):
		command = command.replace( self.str_capacity, str( self.capacity))
		print('Capacity coefficient %s applied:' % str( self.capacity))
		print(command)
		return command

	def applycmdhosts( self, command):
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

	def checkfiles( self, sizemin, sizemax):
		print('Checking for "'+self.files+'" '+str(sizemin)+'-'+str(sizemax))
		if self.files == '':
			print('Error: service::checkfiles: Files not set!')
			return False
		return True
	
	def generatethumbnail(self):
		print("Generating thumbnails")
		files_list = self.files.decode("utf-8").split(";")
		thumbnail_extensions = cgruconfig.VARS['af_thumbnail_extensions']
		
		for filename in files_list:
			self.taskInfo['filename'] = filename
			
			basename, ext = os.path.splitext(os.path.basename(filename))
			if ext in thumbnail_extensions:
				self.taskInfo['thumbnail_filename'] = "%s.jpg" % (basename)
				
				tmp_path = "%s/thumbnails" % cgruconfig.VARS['af_tempdirectory']
				self.taskInfo['thumbnail_tmp_filepath'] = os.path.join(tmp_path, self.taskInfo['thumbnail_filename'])
				thumbnail_path = cgruconfig.VARS['af_thumbnail_naming'] % self.taskInfo
				http_address = "%s%s" % ( cgruconfig.VARS['af_thumbnail_http'], thumbnail_path)
				
				convert_command = cgruconfig.VARS['af_thumbnail_cmd'] % self.taskInfo
				if not os.path.exists(tmp_path):
					os.makedirs(tmp_path)
					
				os.system(convert_command)
				
				match = re.match("(\w+)://([\w\d:]+)(/.*)", http_address)
				if match:
					protocol, servername, http_path = match.groups()
					port = 80
					if servername.find(":") != -1:
						servername, port = servername.split(":")
					
					dav_server = http_client.HTTPConnection(servername, port)
					
					http_dir_list = http_path.split('/')
					http_base_name = http_dir_list.pop()
					http_dir = "%s" % http_dir_list.pop(0)
					for dir_entry in http_dir_list:
						http_dir += "/%s" % dir_entry
						dav_server.request('MKCOL', http_dir)
						dav_response = dav_server.getresponse()
						dav_server.close()
					
					f = open(self.taskInfo['thumbnail_tmp_filepath'], 'rb')
					dav_server.request('PUT', http_path, f.read())
					dav_response = dav_server.getresponse()
					f.close()
				
				os.unlink(self.taskInfo['thumbnail_tmp_filepath'])
	
	def doPost(self):
		def check_flag(byte, flag_name):
			flags = {
					'numeric': 0x01,
					'thumbnails': 0x64
					}
			if flags[flag_name]:
				mask = flags.get(flag_name)
				return byte & mask
			else:
				return 0

		errorMessage = ""
		print("Doing post process")
		if check_flag(self.taskInfo.get('block_flags', 0), "thumbnails"):
			self.generatethumbnail()
		return errorMessage

