from parsers import parser

import re

re_frame = re.compile(r'SCEN.*(progr: begin scene preprocessing for frame )([0-9]{1,})')
re_percent = re.compile(r'JOB.*progr:([ ]{,})([0-9]{1,2}.*)(%[ ]{,})(rendered on).*')
re_number = re.compile(r'[0-9]{1,}')


class maya_mental(parser.parser):
	'Maya To Mental Ray plugin'
	def __init__( self, frames = 1):
		parser.parser.__init__( self, frames)
		self.buffer = ""
		self.numinseq = 0
		
	def do( self, data):
		self.buffer += data
		needcalc = False
		# frame = False
		
		match = re_frame.findall(self.buffer)
		if match:
			try:
				num = int(match[-1][1])
				if self.numinseq < num:
					self.numinseq = num
					self.frame += 1
			except: pass
			
			
			
		match = re_percent.findall(self.buffer)
		if match:
			try: self.percentframe = float(match[-1][1])
			except: pass
			
		'''
		match = re_frame.search(self.buffer)
		if match != None:
			needcalc = True
			frame = True
		else:
			match = re_percent.search(self.buffer)
			if match != None:
				#print(match.groups('2'))
				needcalc = True

		
		if not needcalc: return

		match = re_number.search( match.group(0))
		#print(int( match.group(0)))
		if match == None: return

		try: number = int( match.group(0))
		except: needcalc = False

		
		if not needcalc: return

		if frame:
			if not self.firstframe: self.frame += 1
			self.firstframe = False
		else: self.percentframe = number
		'''
		self.calculate()
