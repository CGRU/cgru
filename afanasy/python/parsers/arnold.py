from parsers import parser

import re
#re.findall(r'(\|\s*)(\d*)(%\s*done)','12:34:56      32MB   |   45% done - 23 rays/pixel')
# |     0% done - 37 rays/pixel
re_percent = re.compile(r'(\|\s*)(\d*)(%\s*done)')

class arnold(parser.parser):
	'Arnold'
	def __init__( self, frames = 1):
		parser.parser.__init__( self, frames)
		self.firstframe = True
		self.data_all = ''

	def do( self, data, mode):
		if len( data ) < 1: return
		
		match = re_percent.findall( data )
		if len( match ):
			percentframe = float( match[-1][1] )
			self.percent = int( percentframe )
