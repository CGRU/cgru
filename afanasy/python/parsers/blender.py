from parsers import parser

import os

# Fra:1 Mem:8.55M (11.55M, peak 29.22M) | Scene, Part 1-16
# Saved: 'render/mypic.0001.jpg'

keyframe = 'Fra:'


class blender(parser.parser):
	"""Blender Batch
	"""

	def __init__(self):
		parser.parser.__init__(self)
		self.str_error = [
			"Warning: Unable to open",
			"Render error: cannot save",
			"Error: CUDA error"]
		self.firstframe = True
		self.framestring = keyframe

	def do(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""
		lines = data.split('\n')
		need_calc = False

		for line in lines:

			if line.find('Saved: ') != -1:
				line = line[6:]
				self.appendFile( line.strip(' \'"`;:,()[]<>{}'))
				continue

			if line.find(keyframe) < 0:
				continue

			frmpos = line.find(' ')

			if frmpos < 0:
				continue

			# Increment frame if new:
			if line[0:frmpos] != self.framestring:
				self.framestring = line[0:frmpos]
				need_calc = True
				if self.firstframe:
					self.firstframe = False
				else:
					self.frame += 1
					self.percentframe = 0

		if need_calc:
			self.calculate()
