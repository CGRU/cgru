from parsers import parser

import re

re_frame   = re.compile(r'Frame ([0-9]{1,})(: caching geometry:)')

# NOTE: This is heavily based on the 'xsi' parser.
# NOTE: This does not support 'percentframe', 
#       this could be added in the future, but 
#       is not the highest priority.
class maya_delight(parser.parser):
   '3Delight For Maya plugin'
   def __init__( self, frames = 1):
      parser.parser.__init__( self, frames)
      self.firstframe = True

   def do( self, data):
      needcalc = False
      frame    = False

      match = re_frame.search( data)
      if match != None:
         needcalc = True
         frame    = True

      if not needcalc: return
      
      if frame:
         if not self.firstframe: self.frame += 1
         self.firstframe = False
      else: 
         self.percentframe = 0.0
      
      self.calculate()
