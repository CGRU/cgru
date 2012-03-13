import parser

keyframe = 'Fra:'

class blender(parser.parser):
   'Blender batch'
   def __init__( self, frames = 1):
      parser.parser.__init__( self, frames)
      self.firstframe = True
      self.framestring = keyframe

   def do( self, data):
      lines = data.split('\n')
      for line in lines:
#         print line
         if line.find( keyframe) > -1:
            frmpos = line.find(' ')
            if frmpos < 0: continue
            if line[0:frmpos] == self.framestring: continue
            self.framestring = line[0:frmpos]
            if self.firstframe:
               self.firstframe = False
            else:
               self.frame += 1
            self.percentframe = 0
            self.calculate()
