from parsers import parser

keyframe = 'RopNode.render: frame '

class hbatch(parser.parser):
   'Houdini9.x batch'
   def __init__( self, frames = 1):
      parser.parser.__init__( self, frames)
      self.firstframe = True

   def do( self, data):

      if data.find('No licenses could be found to run this application') != -1: self.error = True
      if data.find('Please check for a valid license server host') != -1: self.error = True

      if data.find( keyframe) > -1:
         if self.firstframe:
            self.firstframe = False
         else:
            self.frame += 1
         self.percentframe = 0
         self.calculate()
