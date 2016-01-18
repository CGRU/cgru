# -*- coding: utf-8 -*-

from parsers import parser

import re

Warnings	= []
Errors		= []
Badresults	= ['Output file is empty']
Finisheds	= []

re_time		= re.compile(r'(\d{2}):(\d{2}):(\d{2}).(\d{2})')
re_position	= re.compile(r'time=(\d{2}:\d{2}:\d{2}.\d{2})')
re_duration	= re.compile(r'Duration: (\d{2}:\d{2}:\d{2}.\d{2})')

class ffmpeg(parser.parser):
	"""ffmpeg command parser
	"""

	def __init__(self):
		parser.parser.__init__(self)
		self.duration		= 0.1

	def parseTime(self, time):
		res = re_time.match(time)
		if not res:
			return 0
		hours, minutes, seconds, cents = res.groups()
		time = int(hours) * 3600 + int(minutes) * 60 + int(seconds) + int(cents) * 0.01
		return time

	def do(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""

		self.warning = False
		self.error = False
		self.badresult = False
		for warning in Warnings:
			if data.find(warning) != -1:
				self.warning = True
				break
		for error in Errors:
			if data.find(error) != -1:
				self.error = True
				break
		for badresult in Badresults:
			if data.find(badresult) != -1:
				self.badresult = True
				break
		for finished in Finisheds:
			if data.find(finished) != -1:
				self.finishedsuccess = True
				break

		res = re_duration.findall(data)
		if len(res):
			self.duration = self.parseTime(res[0])

		res = re_position.findall(data)
		if len(res):
			time = self.parseTime(res[-1])
			self.percentframe = time / self.duration * 100

			self.calculate()