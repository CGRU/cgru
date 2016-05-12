from parsers import parser

import re

#WORKING!

re_percent = re.compile(r'(mb\s*)(\d*)(%\s*done)')
#re.findall(r'(mb\s*)(\d*)(%\s*done)','INFO : [arnold] 01:15:46 7148mb    55% done - 771 rays/pixel')

class xsi_arnold(parser.parser):
   'Arnold'
   def __init__( self, frames = 1):
      parser.parser.__init__( self, frames)
      self.firstframe = True

   def do( self, data):
      needcalc = True
      frame    = True
 
	 
      match = re_percent.findall( data)
      if len( match ):
        percent = float( match[-1][1] )
        self.percentframe = int( percent )
        self.calculate()
