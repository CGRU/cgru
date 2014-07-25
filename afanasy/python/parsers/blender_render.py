# -*- coding: utf-8 -*-

from parsers import blender

keypart = 'Part '


class blender_render(blender.blender):
	"""Blender Render
	"""

	def __init__(self):
		blender.blender.__init__(self)

	def do(self, data, mode):
		lines = data.split('\n')
		need_calc = False
		for line in lines:
			# print( line)
			ptpos = line.find(keypart)
			if ptpos > 0:
				parts = line[ptpos + 5:].split('-')
				if len(parts) == 2:
					ok = True
					try:
						part0 = int(parts[0])
						part1 = int(parts[1])
					except:  # TODO: too broad exception clause
						ok = False
					if ok:
						if part1 > 0:
							self.percentframe = int(100 * part0 / part1)
							need_calc = True

		if need_calc:
			self.calculate()

		blender.blender.do(self, data, mode)
