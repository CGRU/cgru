# -*- coding: utf-8 -*-

import json, sys

from services import service

class events(service.service):
	'Events Trigger'
	def __init__( self, taskInfo):
		service.service.__init__( self, taskInfo)
		data = self.command
		self.command = ''
		print('Event data:\n%s' % data)
		try:
			if not isinstance( data, str):
				data = str( data, 'utf-8')
			obj = json.loads( data)
		except:
			error = str(sys.exc_info()[1])
			print( error)
			obj = None

		if obj is None: return

		print( json.dumps( obj))

#		self.command = 'mailto.py'

