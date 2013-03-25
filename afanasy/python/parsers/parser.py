import sys

str_warning = '[ PARSER WARNING ]'
str_error = '[ PARSER ERROR ]'
str_badresult = '[ PARSER BAD RESULT ]'
str_finishedsuccess = '[ PARSER FINISHED SUCCESS ]'

class parser:
	"This is base class, not to be instanced"
	def __init__( self, frames = 1):
		self.numframes = frames
		if self.numframes < 1: self.numframes = 1
		self.percent = 0
		self.frame = 0
		self.percentframe = 0
		self.activity = ''
		self.error = False
		self.warning = False
		self.badresult = False
		self.finishedsuccess = False

	def do( self, data):
		print('Error: parser.do: Invalid call, this method must be implemented.')

	def parse( self, data):
		
		if not isinstance( data, str): data = str( data,'utf-8')

		self.activity = ''
		self.warning = False
		self.error = False
		self.badresult = False
		result = None
		if data.find( str_warning         ) != -1: self.warning         = True
		if data.find( str_error           ) != -1: self.error           = True
		if data.find( str_badresult       ) != -1: self.badresult       = True
		if data.find( str_finishedsuccess ) != -1: self.finishedsuccess = True
		try:
			result = self.do( data)
		except:
			print('Error parsing output:')
			print( str(sys.exc_info()[1]))
		return result, self.percent, self.frame, self.percentframe, self.warning, self.error, self.badresult, self.finishedsuccess, self.activity

	def calculate( self):
		if self.frame < 0: self.frame = 0
		if self.frame > self.numframes: self.frame = self.numframes
		if self.percentframe < 0: self.percentframe = 0
		if self.percentframe > 100: self.percentframe = 100
		if self.numframes > 1:
			self.percent = int((100.0*self.frame + self.percentframe)/self.numframes)
		else:
			self.percent = self.percentframe
		if self.percent < 0: self.percent = 0
		if self.percent > 100: self.percent = 100
