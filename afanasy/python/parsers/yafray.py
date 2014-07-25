# -*- coding: utf-8 -*-

from parsers import parser

keyframe = 'Render pass: ['
pass_char = '#'
pass_quantity = 13


class yafray(parser.parser):
	"""Yafray
	"""

	def __init__(self):
		parser.parser.__init__(self)
		self.passing = False
		self.passcount = 0

	def do(self, data, mode):
		"""Missing DocString

		:param data:
		:param mode:
		:return:
		"""
		# print data

		if self.passing:
			# print 'Passing ' + data[:1]
			if data[:1] != pass_char:
				return False
			self.passcount += 1
			if self.passcount >= pass_quantity:
				self.passing = False
			percent = 100 * self.passcount / pass_quantity
			self.percentframe = percent
			self.percent = percent
			return True

		if data.find(keyframe) > -1:
			self.passing = True
			self.passcount = 0

		return False
