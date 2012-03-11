from parsers import parser

keyframe = 'Fra:'

class blender(parser.parser):
   'Blender Batch'
   def __init__( self, frames = 1):
      parser.parser.__init__( self, frames)
      self.firstframe = True
      self.framestring = keyframe

   def do( self, data):
      lines = data.split('\n')
      need_calc = False
      for line in lines:
         if line.find( keyframe) < 0: continue
         frmpos = line.find(' ')
         if frmpos < 0: continue
         # Increment frame if new:
         if line[0:frmpos] != self.framestring:
            self.framestring = line[0:frmpos]
            need_calc = True
            if self.firstframe:
               self.firstframe = False
            else:
               self.frame += 1
               self.percentframe = 0

      if need_calc: self.calculate()
