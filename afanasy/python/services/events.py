# -*- coding: utf-8 -*-
from services import service

parser='generic'

class events(service.service):
	'Events Trigger'
	def __init__( self, taskInfo):
		print('Events service class instanced.')
		service.service.__init__( self, taskInfo)
		print('Command:\n%s' % self.command)
		self.command = ''

