# -*- coding: utf-8 -*-

from parsers import parser

import re
# 00:00:15   325MB         |    20% done - 115 rays/pixel
re_percent = re.compile(r'(\s*)(\d*)(%\s*done)')
re_frame = re.compile(r'Rendering frame [0-9]+')

class xsi_arnold(parser.parser):
	"""Softimage Arnold
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
		
		frame = False
		
		match = re_frame.search(data)
		if match is not None:
			frame = True
			

		if frame:
			if not self.firstframe:
				self.frame += 1
			self.firstframe = False
		else:	
			match = re_percent.findall(data)
			if len(match):
				percentframe = float(match[-1][1])
				self.percent = int(percentframe)
				self.percentframe = int(percentframe)
		
		self.calculate()
