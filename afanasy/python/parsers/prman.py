# -*- coding: utf-8 -*-

from parsers import parser
import re

IMAGE = r'mode = rgba'

class prman(parser.parser):
	"""PIXAR's RenderMan
	"""

	def __init__(self):
		parser.parser.__init__(self)

	def do(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""

		lines = data.split('\n')
		for line in lines:
			pattern = re.compile(IMAGE)
			res = pattern.search(line)
			if res != None:
				quotes = re.split("\"", line)
				if quotes[1] != "":
					self.appendFile(quotes[1].strip())

		# print 'Parsing "prman":'
		# print data
		data_len = len(data)
		# print 'data_len='+str(data_len)
		if data_len > 6:
			if data[data_len - 2] == '%':
				# print '% found:'
				# print data[data_len-5:data_len-2]
				percent = int(data[data_len - 5:data_len - 2])
				if -1 <= percent <= 100:
					self.percentframe = percent
					self.percent = percent
					# print 'PERCENT='+str(self.percent)
					return True
		return False
