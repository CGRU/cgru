# -*- coding: utf-8 -*-

import json
import os
import sys

import cgruconfig

from services import service


# TODO: Class names should follow CamelCase naming convention
class wakeonlan(service.service):
	"""Wake-On-Lan
	"""

	def __init__(self, task_info, i_verbose):
		service.service.__init__(self, task_info, i_verbose)
		data = self.taskInfo['command']
		self.taskInfo['command'] = ''

		#print('Task data:\n%s' % data)

		render = None
		try:
			if not isinstance(data, str):
				data = str(data, 'utf-8')
			render = json.loads(data)
		except:  # TODO: Too broad exception clause
			error = str(sys.exc_info()[1])
			print(error)
			print('Task data:\n%s' % data)
			render = None

		if render is None:
			return

		# Check received render:a
		if not 'address' in render:
			print('ERROR: Received data does not contain address.')
			print('Task data:\n%s' % data)
			return
		if not 'ip' in render['address']:
			print('ERROR: Received data address does not contain IP.')
			print('Task data:\n%s' % data)
			return
		if not 'netifs' in render:
			print('ERROR: Received data does not contain network interfaces info.')
			print('Task data:\n%s' % data)
			return
		if not isinstance(render['netifs'], list):
			print('ERROR: Received netifs is not a list.')
			print('Task data:\n%s' % data)
			return
		if len(render['netifs']) == 0:
			print('ERROR: Received netifs list is empty.')
			print('Task data:\n%s' % data)
			return

		# Construct command:
		if self.taskInfo['wdir'] == 'SLEEP':
			cmd = cgruconfig.VARS['af_render_cmd_wolsleep']
			cmd += ' --sleep'
		else:
			cmd = cgruconfig.VARS['af_render_cmd_wolwake']
			cmd += ' --wake'
		cmd += ' --ip "%s"' % render['address']['ip']
		cmd += ' --hostname "%s"' % render['name']

		for netif in render['netifs']:
			cmd += ' %s' % netif['mac'].replace(':','')

		# print(cmd)
		# Set task command:
		self.taskInfo['command'] = cmd

