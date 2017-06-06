# -*- coding: utf-8 -*-

from parsers import parser

import re
# 00:00:15   325MB         |    20% done - 115 rays/pixel
re_percent = re.compile(r'(\s*)(\d*)(%\s*done)')
re_frame = re.compile(r': Rendering frame [0-9]+')
re_skip = re.compile(r'Skipping Frame [0-9]+')
re_exit = re.compile(r'Render completed ')

class xsi_arnold(parser.parser):
	"""Softimage Arnold
	"""

	def __init__(self):
		parser.parser.__init__(self)
		self.firstframe = True
		self.data_all = ''

	def do(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""

		if len(data) < 1:
			return
		
		frame = False
		
		match = re_frame.search(data)
		if match is not None:
			frame = True
		
		if self.percentframe == 0:	
			self.activity = 'loading..'	
		
		
		match = re_skip.findall(data)
		if match is not None:
			if len(match):
				self.activity = 'skipping frames..'
				print("skipping " + str(len(match)) + " frames...")
				self.frame += int(len(match))
				self.calculate()
				return
						
			
		if frame:
			if not self.firstframe:
				self.frame += 1
			self.firstframe = False
		else:	
			match = re_percent.findall(data)
			if len(match):
				percentframe = float(match[-1][1])
				self.percent = int(percentframe)
				self.percentframe = int(percentframe)
				
				# activity
				if self.percentframe < 99:
					self.activity = 'rendering..'	
				if self.percentframe > 99:
					self.activity = 'finalizing..'
		
		match = re_exit.findall(data)
		if match is not None:
			if len(match):
				print("block should be complete...")
				self.percent = 100
				self.percentframe = 100
		
		self.calculate()
