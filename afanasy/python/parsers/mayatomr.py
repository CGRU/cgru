# -*- coding: utf-8 -*-

from parsers import parser

FRAME = 'FRAME: '
PERCENT = 'PROGRESS: '
PERCENT_len = len(PERCENT)


class mayatomr(parser.parser):
	"""Simple generic parser (mayatomr)
	"""

	def __init__(self):
		parser.parser.__init__(self)
		self.firstframe = True

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
				needcalc = True
		percent_pos = data.find(PERCENT)
		if percent_pos > -1:
			ppos = data.find('%')
			if ppos > -1:
				needcalc = True
				self.percentframe = int(data[percent_pos + PERCENT_len:ppos])
		if needcalc:
			self.calculate()
