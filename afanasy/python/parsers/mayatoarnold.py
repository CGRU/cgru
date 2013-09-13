from parsers import parser

FRAME = 'writing scene to'

class mayatoarnold(parser.parser):
	'Maya to Arnold'
	def __init__( self, frames = 1):
		print('mayatoarnold(parser.parser)')
		parser.parser.__init__( self, frames)

	def do( self, data, mode):
		print(data)
		needcalc = False

		if data.find(FRAME) > -1:
			self.frame += 1
			needcalc = True

		if( needcalc ): self.calculate()
