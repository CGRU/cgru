# -*- coding: utf-8 -*-


from parsers import parser

import re

re_frame = re.compile(r'PROGRESS: .* (.*): .* Seconds')


class afterfx(parser.parser):
	"""Adobe After Effects
	"""

	def __init__(self):
		parser.parser.__init__(self)
		self.str_error = [
			'aerender Error',
			'After Effects error',
			'Unable to Render']
		self.firstframe = True
		self.data_all = ''

	def do(self, data, mode):
		self.data_all += data

		# Check whether was any progress:
		if mode == 'finished':
			if self.data_all.find('PROGRESS') == -1:
				self.badresult = True

		match = re_frame.search(data)
		if match is None:
			return

		if not self.firstframe:
			self.frame += 1

		self.firstframe = False
		self.calculate()
