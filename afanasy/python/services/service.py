# -*- coding: utf-8 -*-

import os
import sys
import re

import traceback

import cgruconfig
import cgrupathmap
import cgruutils

str_capacity = '@AF_CAPACITY@'
str_hosts = '@AF_HOSTS@'
str_hostsprefix = '-H '
str_hostseparator = ','


class service(object):  # TODO: Class names should follow CamelCase naming convention
	"""This is base service class.

	:param taskInfo:
	"""

	def __init__(self, taskInfo, i_verbose):
		self.taskInfo = taskInfo
		self.verbose = i_verbose

		self.pm = cgrupathmap.PathMap()

		self.str_capacity = str_capacity
		self.str_hosts = str_hosts
		self.str_hostsprefix = str_hostsprefix
		self.str_hostseparator = str_hostseparator

		# Transfer command and working folder:
		command = taskInfo['command']
		command = self.pm.toClient(command)

		# Apply capacity:
		if self.taskInfo['capacity'] > 0:
			command = self.applyCmdCapacity(command)

		# Apply hosts (multihosts tasks):
		if len(self.taskInfo['hosts']):
			command = self.applyCmdHosts(command)
		taskInfo['command'] = command
		taskInfo['wdir'] = self.pm.toClient(taskInfo['wdir'])

		for i in range(0, len(self.taskInfo['files'])):
			self.taskInfo['files'][i] = \
				self.pm.toClient(self.taskInfo['files'][i])

		# When GUI receives task exec to show files,
		# server sends exec with parsed files.
		for i in range(0, len(self.taskInfo['parsed_files'])):
			self.taskInfo['parsed_files'][i] = \
				self.pm.toClient(self.taskInfo['parsed_files'][i])

		# Initialize parser:
		self.parser = None
		parser = cgruutils.toStr(taskInfo['parser'])
		if len(taskInfo['parser']):
			try:
				mod = __import__('parsers', globals(), locals(), [parser])
				cmd = 'mod.%s.%s()' % (parser, parser)
				self.parser = eval(cmd)
				self.parser.setTaskInfo(taskInfo)
			except:  # TODO: too broad exception clause
				self.parser = None
				print('ERROR: Failed to import parser "%s"' % parser)
				traceback.print_exc(file=sys.stdout)

		if self.verbose:
			print(taskInfo)


	def getWDir(self):
		"""Missing DocString

		:return:
		"""
		return self.taskInfo['wdir']

	def getCommand(self):
		"""Missing DocString

		:return:
		"""
		return self.taskInfo['command']

	def getFiles(self):
		"""Missing DocString

		:return:
		"""
		return self.taskInfo['files']

	def getParsedFiles(self):
		"""Missing DocString

		:return:
		"""
		# taskInfo does not have parsed files on render,
		# afserver set parsed files parameter on TaskExec for GUIs only,
		# it needed for GUIs only to transfer files paths to view
		if len(self.taskInfo['parsed_files']):
			return self.taskInfo['parsed_files']
		elif self.parser is not None:
			files = self.parser.getFiles()
			for i in range(0, len(files)):
				files[i] = self.pm.toServer(files[i])
			return files
		else:
			return []

	def applyCmdCapacity(self, command):
		"""Missing DocString

		:return:
		"""
		command = command.replace(
			self.str_capacity,
			str(self.taskInfo['capacity'])
		)
		print('Capacity coefficient %s applied:' % self.taskInfo['capacity'])
		print(command)
		return command

	def applyCmdHosts(self, command):
		"""Missing DocString

		:param command:
		:return:
		"""
		hosts = str_hostsprefix
		firsthost = True
		for host in self.taskInfo['hosts']:
			if firsthost:
				firsthost = False
			else:
				hosts += self.str_hostseparator
			hosts += host
		command = command.replace(self.str_hosts, hosts)
		print('Hosts list "%s" applied:' % str(hosts))
		print(command)
		return command

	def parse(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""
		if self.parser is None:
			return None

		thumb_cmds = self.generateThumbnail( True)
		for cmd in thumb_cmds:
			print('Generating thumbnail "on-the-fly":')
			print( cmd)
			os.system(cmd)

		return self.parser.parse(data, mode)

	def checkExitStatus(self, i_status):
		""" This function needed to check task process exit status.
			By default zero is success, any other means some error.
			But some services can have some other good exit status value(s).
		"""
		status = False
		if i_status == 0:
			status = True

		if self.verbose:
			msg = 'ERROR'
			if status:
				msg = 'SUCCESS'
			print('service::checkExitStatus: %d %s' % (i_status, msg))

		return status

	def doPost(self):
		"""Missing DocString

		:return:
		"""
		# def check_flag(byte, flag_name):
		# return True
		# flags = {
		#					'numeric': 0x01,
		#					'thumbnails': 0x64
		#					}
		#			if flags[flag_name]:
		#				mask = flags.get(flag_name)
		#				return byte & mask
		#			else:
		#				return 0

		post_cmds = []
		#print( self.parser.getFiles())
		# if len( self.taskInfo['files']) and check_flag( self.taskInfo.get('block_flags', 0), 'thumbnails'):
		post_cmds.extend(self.generateThumbnail( False))
		# post_cmds.extend(['ls -la > ' + self.taskInfo['store_dir'] + '/afile'])
		return post_cmds

	def generateThumbnail(self, i_onthefly):
		"""Missing DocString

		:return:
		"""
		cmds = []

		if not os.path.isdir(self.taskInfo['store_dir']):
			return cmds

		files_list = []
		if self.parser is not None:
			if i_onthefly:
				files_list = self.parser.getFilesOnTheFly()
			else:
				files_list = self.parser.getFiles()

		if len(files_list):
			if len(files_list) > 3:
				files_list = [
					files_list[0],
					files_list[int(len(files_list) / 2)],
					files_list[-1]
				]
		elif len(self.taskInfo['files']) and not i_onthefly:
			for afile in self.taskInfo['files']:
				files_list.append(afile)
			# files_list.append(afile.decode('utf-8'))
		else:
			return cmds

		for image in files_list:
			image = cgruutils.toStr(image)
			if len(image) < 1:
				continue
			image = os.path.join(self.taskInfo['wdir'], image)
			if not os.path.isfile(image):
				continue

			basename, ext = os.path.splitext(os.path.basename(image))
			if len(ext) < 2:
				continue
			ext = ext.lower()[1:]
			if not ext in cgruconfig.VARS['af_thumbnail_extensions']:
				continue

			store_dir = cgruutils.toStr(self.taskInfo['store_dir'])
			thumbnail = os.path.basename(image) + '.jpg'
			thumbnail = store_dir + '/' + thumbnail

			self.taskInfo['image'] = os.path.normpath(image)
			self.taskInfo['thumbnail'] = os.path.normpath(thumbnail)
			self.taskInfo['pre_args'] = ''
			if ext == 'dpx' or ext == 'cin':
				self.taskInfo['pre_args'] = '-set colorspace Log'
			if ext == 'exr':
				self.taskInfo['pre_args'] = '-set colorspace RGB'

			cmd = str(cgruconfig.VARS['af_thumbnail_cmd']) % self.taskInfo
			# print( cmd)
			# cmds.append('echo ' + cmd)
			cmds.append(cmd)

		return cmds

	# Not used:
	def checkfiles(self, sizemin, sizemax):
		"""Missing DocString

		:param sizemin:
		:param sizemax:
		:return:
		"""
		print(
		'Checking for "%s" %s-%s' %
		(self.taskInfo['files'], sizemin, sizemax)
		)

		if self.taskInfo['files'] == '':
			print('Error: service::checkfiles: Files not set!')
			return False

		return True

