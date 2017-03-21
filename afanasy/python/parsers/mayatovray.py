# -*- coding: utf-8 -*-

from parsers import parser

FRAME = 'Appending '
RESET_COUNT = 'Render complete'
PERCENT = 'PROGRESS: '
PERCENT_len = len(PERCENT)

class mayatovray(parser.parser):
	"""Simple generic parser (mayatovray)
	"""

	def __init__(self):
		parser.parser.__init__(self)
		self.firstframe = True
		self.layer = 1

	def do(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""
		# print(data)
		needcalc = False
		if data.find(FRAME) > -1:
			if self.firstframe:
				self.firstframe = False
			else:
				self.frame += 1
				self.activity = 'Layer ' + str(self.layer) + ':'
				needcalc = True
		if data.find(RESET_COUNT) > -1:
			self.layer += 1
			self.firstframe = True
			self.frame = 0
		percent_pos = data.find(PERCENT)
		if percent_pos > -1:
			ppos = data.find('%')
			if ppos > -1:
				needcalc = True
				self.percentframe = int(data[percent_pos + PERCENT_len:ppos])
		if needcalc:
			self.calculate()
