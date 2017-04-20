# -*- coding: utf-8 -*-

from parsers import parser

import re

re_progress	= re.compile(r'([\d\.:]+\w?)\s+([\d\.:]+\w?)\s+([\d\.:]+\w?)\s+([\d\.:]+\w?)\s+([\d\.:]+\w?)\s+([\d\.:]+\w?)\s+([\d\.:]+\w?)\s+([\d\.:]+\w?)\s+([\d\.:]+\w?)\s+([\d\.:]+\w?)\s+([\d\.:]+\w?)\s+([\d\.:]+\w?)')

class curl(parser.parser):
	"""curl command parser
	"""

	def __init__(self):
		parser.parser.__init__(self)

	def do(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""

		data = re_progress.findall(data)
		if len(data) == 0:
			return

		percent = data[0][0]
		self.percentframe = int(percent)

		self.calculate()
