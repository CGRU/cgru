# -*- coding: utf-8 -*-

import cgruconfig

import json, os, sys

from services import service

class events(service.service):
	'Events Trigger'
	def __init__( self, task_info):
		service.service.__init__( self, task_info)
		data = self.command
		self.command = ''
		#print('Event data:\n%s' % data)
		try:
			if not isinstance( data, str):
				data = str( data, 'utf-8')
			objects = json.loads( data)
		except:
			error = str(sys.exc_info()[1])
			print( error)
			print('Event data:\n%s' % data)
			objects = None

		if objects is None: return

		# Check recieved events:
		if not 'events' in objects:
			print('ERROR: Recieved data does not contain events.')
			print('Event data:\n%s' % data)
			return
		if not isinstance(objects['events'], list):
			print('ERROR: Recieved events is not a list.')
			print('Event data:\n%s' % data)
			return
		if len(objects['events']) == 0:
			print('ERROR: Recieved events list is empty.')
			print('Event data:\n%s' % data)
			return

		# Combine objects:
		obj = {}
		# Get object from user in any:
		if 'user' in objects: obj = objects['user']
		# Update with job object in any:
		if 'job' in objects: obj.update( objects['job'])

		if len( obj ) == 0:
			#print('No configured data founded.')
			return

		if not 'events' in obj:
			#print('No configured events founded.')
			return

		email_events = []

		# Iterate all interested events:
		for event in obj['events']:

			if not event in objects['events']:
				#print('Skipping not recieved event "%s"' % event)
				continue

			event_obj = obj['events'][event]

			# Event should be a dictionary:
			if not isinstance( event_obj, dict):
				print('ERROR: Configured event["%s"] is not an object.' % event )
				print('Event data:\n%s' % data)
				return

			if not 'methods' in event_obj:
				print('ERROR: Configured event["%s"] does not have methods.' % event )
				print('Event data:\n%s' % data)
				continue

			methods = event_obj['methods']
			if not isinstance( methods, list):
				print('ERROR: Configured event["%s"] methods is not an array.' % event )
				print('Event data:\n%s' % data)
				continue

			if 'email' in methods and 'email' in obj:
				print('EVENT: %s:%s %s:%s' % ( event, task_info['job_name'], task_info['user_name'], obj['email'] ))
				email_events.append( event)

		if len(email_events):
			cmd = cgruconfig.VARS['email_send_cmd']
			cmd += ' -V' # Verbose mode
			cmd += ' -f "noreply@%s"' % cgruconfig.VARS['email_sender_address_host']
			cmd += ' -t "%s"' % obj['email']
			cmd += ' -s "%s"' % (','.join( email_events))
			cmd += ' "Events: %s<br>"' % (','.join( email_events))
			cmd += ' "User Name: %s<br>"' % task_info['user_name']
			cmd += ' "Job Name: %s"' % task_info['job_name']
			print(cmd)
			self.command = cmd

