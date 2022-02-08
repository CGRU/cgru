# -*- coding: utf-8 -*-

from parsers import parser


Errors = ['FTRACK ERROR']


class shotgun(parser.parser):
	"""Simple shotgun  parser
	"""
	def __init__(self):
		parser.parser.__init__(self)
		self.firstframe = True

	def do(self, data, mode):
		for error in Errors:
			if data.find(error) != -1:
				self.error = True
				#self.badresult = True
				break

