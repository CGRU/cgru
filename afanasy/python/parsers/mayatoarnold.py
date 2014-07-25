# -*- coding: utf-8 -*-

from parsers import parser

FRAME = 'writing scene to'


class mayatoarnold(parser.parser):
	"""Maya to Arnold
	"""

	def __init__(self):
		print('mayatoarnold(parser.parser)')
		parser.parser.__init__(self)

	def do(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""
		# print(data)
		needcalc = False

		if data.find(FRAME) > -1:
			self.frame += 1
			needcalc = True

		if needcalc:
			self.calculate()
