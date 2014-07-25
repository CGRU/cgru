# -*- coding: utf-8 -*-

from parsers import blender
from parsers import yafray


class blender_yafray(blender.blender, yafray.yafray):
	"""Blender batch + catch yafray output
	"""

	def __init__(self):
		blender.blender.__init__(self)
		yafray.yafray.__init__(self)

	def do(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""
		# print data
		if yafray.yafray.do(self, data):
			self.calculate()

		blender.blender.do(self, data, mode)
