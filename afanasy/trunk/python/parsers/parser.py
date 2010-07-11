class parser:
   "This is base class, not to be instanced"
   def __init__( self, frames = 1):
      self.numframes = frames
      if self.numframes < 1: self.numframes = 1
      self.percent = 0
      self.frame = 1
      self.percentframe = 0
      self.error = False
      self.warning = False
      self.badresult = False

   def do( self, data):
      print 'Erorr: parser.do: Invalid call, this method must be implemented.'

   def parse( self, data):
      self.error = False
      self.warning = False
      self.badresult = False
      result = self.do( data)
#      if data.find('[ BAD RESULT ]') != -1: self.badresult = True
#      self.badresult = True
      return result, self.percent, self.frame, self.percentframe, self.warning, self.error, sself.badresult

   def calculate( self):
      if self.frame < 1: self.frame = 1
      if self.frame > self.numframes: self.frame = self.numframes
      if self.percentframe < 0: self.percentframe = 0
      if self.percentframe > 100: self.percentframe = 100
      if self.numframes > 1:
         self.percent = int((100.0*(self.frame-1) + self.percentframe)/self.numframes)
      else:
         self.percent = self.percentframe
      if self.percent < 0: self.percent = 0
      if self.percent > 100: self.percent = 100
