# -*- coding: utf-8 -*-

from parsers import parser

import re
# re.findall(r'(\|\s*)(\d*)(%\s*done)','12:34:56      32MB   |   45% done - 23 rays/pixel')
# |     0% done - 37 rays/pixel
re_percent = re.compile(r'(\s*)(\d*)(\s*% done)')


class houdinitoarnold(parser.parser):
	"""Houdini to Arnold
	"""

	def __init__(self):
		parser.parser.__init__(self)
		self.firstframe = True
		self.data_all = ''

	def do(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""
		if len(data) < 1:
			return

		match = re_percent.findall(data)
		if len(match):
			percentframe = float(match[-1][1])
			self.percent = int(percentframe)
