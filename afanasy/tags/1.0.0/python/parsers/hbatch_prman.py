#import parser
import hbatch
import prman

class hbatch_prman( hbatch.hbatch, prman.prman):
   'Houdini9.x batch + catch rib node output'
   def __init__( self, frames = 1):
      hbatch.hbatch.__init__( self, frames)
      prman.prman.__init__( self, frames)

   def do( self, data):
      #print data
      if prman.prman.do( self, data): self.calculate()
      hbatch.hbatch.do( self, data)
