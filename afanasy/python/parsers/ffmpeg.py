# -*- coding: utf-8 -*-

from parsers import p

import re

str_warning = '[ PARSER WARNING ]'
str_error = '[ PARSER ERROR ]'
str_badresult = '[ PARSER BAD RESULT ]'
str_finishedsuccess = '[ PARSER FINISHED SUCCESS ]'

class ffmpeg(parser.parser):
	"""ffmpeg command parser
	"""

	def __init__(self):
		parser.parser.__init__(self)

	def do(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""

		res = re.findall(r'frame= *(\d+)', data)
		if len(res):
			self.frame = int(res[-1]) # Get the last information available

		self.calculate()