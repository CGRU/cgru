from parsers import blender
from parsers import yafray

class blender_yafray( blender.blender, yafray.yafray):
   'Blender batch + catch yafray output'
   def __init__( self, frames = 1):
      blender.blender.__init__( self, frames)
      yafray.yafray.__init__( self, frames)

   def do( self, data):
      #print data
      if yafray.yafray.do( self, data): self.calculate()
      blender.blender.do( self, data)
