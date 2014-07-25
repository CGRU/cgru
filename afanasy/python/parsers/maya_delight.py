# -*- coding: utf-8 -*-

from parsers import parser

import re

re_frame = re.compile(r'Frame ([0-9]+)(: caching geometry:)')

# NOTE: This is heavily based on the 'xsi' parser.
# NOTE: This does not support 'percentframe',
# this could be added in the future, but
# is not the highest priority.


class maya_delight(parser.parser):
	"""3Delight For Maya plugin
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
		needcalc = False
		frame = False

		match = re_frame.search(data)
		if match is not None:
			needcalc = True
			frame = True

		if not needcalc:
			return

		if frame:
			if not self.firstframe:
				self.frame += 1
			self.firstframe = False
		else:
			self.percentframe = 0.0

		self.calculate()
