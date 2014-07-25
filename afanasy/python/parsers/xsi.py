# -*- coding: utf-8 -*-

from parsers import parser

import re

re_frame = re.compile(r'Rendering frame [0-9]+')
re_percent = re.compile(r'progr:[ ]{,}[0-9]{1,2}.*%[ ]{,}rendered')
re_number = re.compile(r'[0-9]+')


class xsi(parser.parser):
	"""Softimage
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
		else:
			match = re_percent.search(data)
			if match is not None:
				needcalc = True

		if not needcalc:
			return

		match = re_number.search(match.group(0))
		if match is None:
			return

		try:
			number = int(match.group(0))
		except:  # TODO: too broad exception clause
			needcalc = False

		if not needcalc:
			return

		if frame:
			if not self.firstframe:
				self.frame += 1
			self.firstframe = False
		else:
			# TODO: number variable might be referenced before assignment
			self.percentframe = number

		self.calculate()
