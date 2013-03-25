from parsers import parser
import re

key = 'Writing'
key_len = len(key)

ErrorsRE = [re.compile(r'Invalid .* license key.')]

class nuke(parser.parser):
	'The Foundry Nuke'
	def __init__( self, frames = 1):
		parser.parser.__init__( self, frames)
		self.filename = ''

	def do( self, data):
		data_len = len(data)
		if data_len < 1: return

		for errorRE in ErrorsRE:
			if errorRE.search( data) is not None:
				self.error = True
				break

		needcalc = False

		key_pos = data.find( key)
		if key_pos > -1:
			file_begin = key_pos+key_len+1
			file_end = data.find( ' ', file_begin, data_len)
			if file_end > 1:
				newfilename = data[ file_begin:file_end]
				if newfilename != '':
					if self.filename != newfilename:
						if self.filename != '':
							self.frame += 1
							self.percentframe = 0
							needcalc = True
						self.filename = newfilename

		if data[data_len-2:data_len-1] == '.':
			char = data[data_len-1:data_len]
			if char != ' ':
				if ord(char) <= ord('9'):
					if ord(char) >= ord('0'):
						self.percentframe = int(char) * 10
						needcalc = True

		activity_pos = data.rfind('EXECUTING VIEW "')
		if activity_pos > -1:
			activity_pos += len('EXECUTING VIEW "')
			self.activity = data[ activity_pos : data.find('"', activity_pos) ]

		if( needcalc ): self.calculate()
