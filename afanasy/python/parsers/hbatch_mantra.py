# -*- coding: utf-8 -*-

from parsers import hbatch
from parsers import mantra


class hbatch_mantra(hbatch.hbatch, mantra.mantra):
	"""Houdini batch + catch mantra node output
	"""

	def __init__(self):
		hbatch.hbatch.__init__(self)
		mantra.mantra.__init__(self)

	def do(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""
		# print data
		mantra.mantra.do(self, data, mode)
		hbatch.hbatch.do(self, data, mode)
