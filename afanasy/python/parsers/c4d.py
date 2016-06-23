# -*- coding: utf-8 -*-

from parsers import parser

import re

re_percent = re.compile(r'Progress: \d+%')
re_frame = re.compile(r'Rendering frame \d+')
re_number = re.compile(r'\d+')


class c4d(parser.parser):
	"""Cinema 4D
	"""

	def __init__(self):
		parser.parser.__init__(self)
		self.str_error = [
			'Files cannot be written - please check output paths!',
			'Error rendering project']
		self.firstFrame = True
		self.firstFrameNumber = 0

	def do(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""
		need_calc = False

		# Search for frame percent:
		match = re_percent.search(data)
		if match is not None:
			frame = re_number.search(match.group(0))
			self.percentframe = int(frame.group(0))
			need_calc = True

		# Search for frame number:
		match = re_frame.search(data)

		if match is not None:
			# Get the current frame-number
			frame = re_number.search(match.group(0))
			thisFrame = int(frame.group(0))

			# To know with what frame the task starts
			if self.firstFrame:
				self.firstFrameNumber = thisFrame
				self.firstFrame = False

			# Calculate the current frame (not the frame-number!)
			self.frame = thisFrame - self.firstFrameNumber

			need_calc = True

		if need_calc:
			self.calculate()
