# -*- coding: utf-8 -*-

from parsers import hbatch
from parsers import prman


class hbatch_prman(hbatch.hbatch, prman.prman):
	"""Houdini batch + catch rib node output
	"""

	def __init__(self):
		hbatch.hbatch.__init__(self)
		prman.prman.__init__(self)

	def do(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""
		# print data
		if prman.prman.do(self, data, mode):
			self.calculate()
		hbatch.hbatch.do(self, data, mode)
