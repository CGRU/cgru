from parsers import parser

FRAME = 'FRAME: '
PERCENT = 'PROGRESS: '
ACTIVITY = 'ACTIVITY: '

PERCENT_len = len(PERCENT)
ACTIVITY_len = len(ACTIVITY)

class generic(parser.parser):
	'Simple generic parser'
	def __init__( self, frames = 1):
		parser.parser.__init__( self, frames)
		self.firstframe = True

	def do( self, data):
		needcalc = False

		if data.rfind(FRAME) > -1:
			if self.firstframe:
				self.firstframe = False
			else:
				self.frame += 1
				needcalc = True

		percent_pos = data.rfind(PERCENT)
		if percent_pos > -1:
			percent_pos += PERCENT_len
			ppos = data.find('%', percent_pos)
			if ppos > -1:
				needcalc = True
				self.percentframe = int(data[percent_pos:ppos])

		activity_pos = data.rfind(ACTIVITY)
		if activity_pos > -1:
			activity_pos += ACTIVITY_len
			self.activity = data[ activity_pos : data.find('\n', activity_pos) ]

		if( needcalc ): self.calculate()
