#import parser
import hbatch
import mantra

class hbatch_mantra( hbatch.hbatch, mantra.mantra):
   'Houdini9.x batch + catch mantra node output'
   def __init__( self, frames = 1):
      hbatch.hbatch.__init__( self, frames)
      mantra.mantra.__init__( self, frames)

   def do( self, data):
      #print data
      if mantra.mantra.do( self, data): self.calculate()
      hbatch.hbatch.do( self, data)
