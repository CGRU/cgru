# -*- coding: utf-8 -*-

from parsers import parser

import re
# INFO : [Redshift] 	Block 32/48 (7,4) rendered by GPU 0 in 2ms
re_percent = re.compile(r'(Block*)(\s*)(\d*)(\/)(\d*)(\s*)(\S*)(\s*)(rendered by GPU)')
re_frame = re.compile(r': Rendering frame [0-9]+')
re_skip = re.compile(r'[0-9]+ skipped')
re_exit = re.compile(r'Render completed ')

class xsi_redshift(parser.parser):
	"""Softimage Redshift
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
		block = 0
		blockCount = 0
		
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

			
		#print("cur frame " + str(self.frame))


		match = re_percent.findall(data)
		if match is not None:
			if len(match):
				# get current block
				block = float(match[0][2])
				# get blockCount
				blockCount = float(match[0][4])

				# calculate percentage
				percentframe = float(100/(blockCount/block))

				#print(str(percentframe))
				
				#somewhat a hack to avoid jumping percentages with multiple GPUs
				if int(percentframe) > 20:
					if int(percentframe) > self.percentframe:
						self.percent = int(percentframe)
						self.percentframe = int(percentframe)
				else:
					self.percent = int(percentframe)
					self.percentframe = int(percentframe)
					
				#print("Redshift: " + str(self.percentframe) + "%" )
					
				# activity
				if self.percentframe < 99:
					self.activity = 'rendering..'
				if self.percentframe > 99:
					self.activity = 'finalizing..'

		
		if frame:
			if not self.firstframe:
				self.frame += 1

			self.firstframe = False

			
		match = re_exit.findall(data)
		if match is not None:
			if len(match):
				print("block should be complete...")
				self.percent = 100
				self.percentframe = 100
			
		self.calculate()
