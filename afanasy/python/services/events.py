# -*- coding: utf-8 -*-

import json
import os
import sys

import cgruconfig

from services import service


# TODO: Class names should follow CamelCase naming convention
class events(service.service):
	"""Events Trigger
	"""

	def __init__(self, task_info, i_verbose):
		service.service.__init__(self, task_info, i_verbose)
		data = self.taskInfo['command']
		self.taskInfo['command'] = ''

		# print('Event data:\n%s' % data)

		try:
			if not isinstance(data, str):
				data = str(data, 'utf-8')
			objects = json.loads(data)
		except:  # TODO: Too broad exception clause
			error = str(sys.exc_info()[1])
			print(error)
			print('Event data:\n%s' % data)
			objects = None

		if objects is None:
			return

		# Check received events:
		if not 'events' in objects:
			print('ERROR: Received data does not contain events.')
			print('Event data:\n%s' % data)
			return
		if not isinstance(objects['events'], list):
			print('ERROR: Received events is not a list.')
			print('Event data:\n%s' % data)
			return
		if len(objects['events']) == 0:
			print('ERROR: Received events list is empty.')
			print('Event data:\n%s' % data)
			return

		# Combine objects:
		obj = dict()
		# Update with user custom object in any:
		if 'custom_data' in objects['user']:
			try:
				obj.update(json.loads(objects['user']['custom_data']))
			except:  # TODO: too broad exception clause
				print('JSON error in user custom data:')
				print(objects['user']['custom_data'])
				print(sys.exc_info()[1])
				return
		# Update with job custom object in any:
		if 'custom_data' in objects['job']:
			try:
				obj.update(json.loads(objects['job']['custom_data']))
			except:  # TODO: too broad exception clause
				print('JSON error in job custom data:')
				print(objects['job']['custom_data'])
				print(sys.exc_info()[1])
				return

			# print('Custom data:')
		# print(json.dumps(obj))

		if len(obj) == 0:
			#print('No configured data found.')
			return

		if not 'events' in obj:
			#print('No configured events found.')
			return

		email_events = []

		# Iterate all interested events:
		for event in obj['events']:

			if not event in objects['events']:
				#print('Skipping not received event "%s"' % event)
				continue

			event_obj = obj['events'][event]

			# Event should be a dictionary:
			if not isinstance(event_obj, dict):
				print('ERROR: Configured event["%s"] is not an object.' % event)
				print('Event data:\n%s' % data)
				return

			if not 'methods' in event_obj:
				print('ERROR: Configured event["%s"] does not have methods.' % event)
				print('Event data:\n%s' % data)
				continue

			methods = event_obj['methods']
			if not isinstance(methods, list):
				print('ERROR: Configured event["%s"] methods is not an array.' % event)
				print('Event data:\n%s' % data)
				continue

			if 'email' in methods and 'email' in obj:
				print('EVENT: %s:%s %s:%s' %
					  (event, task_info['job_name'], task_info['user_name'],
					   obj['email'])
				)
				email_events.append(event)

			# Essentially for debugging
			if 'notify-send' in methods:
				self.taskInfo['command'] = "notify-send Afanasy 'Job " + task_info['job_name'].replace("'", "'\\''") + ": " + event + "'"

		if len(email_events):
			cmd = cgruconfig.VARS['email_send_cmd']
			cmd += ' -V'  # Verbose mode
			cmd += ' -f "noreply@%s"' % cgruconfig.VARS[
				'email_sender_address_host']
			cmd += ' -t "%s"' % obj['email']
			cmd += ' -s "%s"' % (','.join(email_events))
			cmd += ' "Events: %s<br>"' % (','.join(email_events))
			cmd += ' "User Name: %s<br>"' % task_info['user_name']
			cmd += ' "Job Name: %s"' % task_info['job_name']
			print(cmd)
			self.taskInfo['command'] = cmd
