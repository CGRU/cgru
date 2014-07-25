# -*- coding: utf-8 -*-

from parsers import parser

import re


class rsync(parser.parser):
	"""Rsync command parser
	"""

	def __init__(self):
		parser.parser.__init__(self)

	# print('rsync parser instanced')

	def do(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""
		# print(data)

		data = re.findall(r'\d*%', data)
		if len(data) == 0:
			return

		data = data[0]
		if data.find('%') <= 0:
			return

		data = data.replace('%', '')
		self.percentframe = int(data)

		# print('percent = ' + data)

		self.calculate()
