import parser

keyframe = 'RopNode.render: frame '

class hbatch(parser.parser):
   'Houdini9.x batch'
   def __init__( self, frames = 1):
      parser.parser.__init__( self, frames)
      self.firstframe = True

   def do( self, data):
      if data.find( keyframe) > -1:
         if self.firstframe:
            self.firstframe = False
         else:
            self.frame += 1
         self.percentframe = 0
         self.calculate()
