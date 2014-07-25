# -*- coding: utf-8 -*-

from parsers import parser

import re

re_frame = re.compile(
	r'SCEN.*(progr: begin scene preprocessing for frame )([0-9]+)'
)

re_percent = re.compile(
	r'JOB.*progr:([ ]{,})([0-9]{1,2}.*)(%[ ]{,})(rendered on).*'
)

re_number = re.compile(r'[0-9]+')


class mentalray(parser.parser):
	"""Mental Ray Standalone
	"""

	def __init__(self):
		parser.parser.__init__(self)
		self.buffer = ""
		self.numinseq = 0

	def do(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""
		# self.buffer += data
		# needcalc = False
		# frame = False

		if len(data) < 1:
			return

		match = re_percent.findall(data)
		if len(match):
			percentframe = float(match[-1][1])
			self.percent = int(percentframe)
