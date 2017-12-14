# -*- coding: utf-8 -*-

from parsers import parser

import re

re_time				= re.compile(r'(\d{2}):(\d{2}):(\d{2}).(\d{2})')

re_duration			= re.compile(r'Duration: *(\d{2}:\d{2}:\d{2}.\d{2})')
re_frames			= re.compile(r'Frames: *(\d+)')
re_position_time	= re.compile(r'time=(\d{2}:\d{2}:\d{2}.\d{2})')
re_position_frame	= re.compile(r'frame= *(\d+) ')

class ffmpeg(parser.parser):
	"""ffmpeg command parser
	"""

	def __init__(self):
		parser.parser.__init__(self)
		self.str_badresult	= ['Output file is empty']
		self.str_error		= ['Invalid', 'Impossible to open', 'No such file or directory']
		self.duration		= 0.1
		self.frame			= 0
		self.frames			= 0

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

		res = re_frames.findall(data)
		if len(res):
			self.frames = int(res[0])

		res = re_duration.findall(data)
		if len(res):
			self.duration = self.parseTime(res[0])

		position_frame	= 0
		position_time	= 0
		if ( self.frames ):
			res = re_position_frame.findall(data)
			if len(res):
				position_frame = int(res[0])
		else:
			res = re_position_time.findall(data)
			if len(res):
				position_time = self.parseTime(res[-1])

		if ( position_frame ):
			self.percentframe = int(position_frame / float(self.frames) * 100)
			self.calculate()

		if ( position_time ):
			self.percentframe = int(position_time / float(self.duration) * 100)
			self.calculate()