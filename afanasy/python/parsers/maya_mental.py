from parsers import parser

import re

re_frame   = re.compile(r'SCEN.*(progr: begin scene preprocessing for frame )([0-9]{1,})')
re_percent = re.compile(r'JOB.*progr:([ ]{,})([0-9]{1,2}.*)(%[ ]{,})(rendered on).*')
re_number  = re.compile(r'[0-9]{1,}')

# NOTE: This is basically a copy-paste from the 'xsi' parser, 
#       the only things changed were the regular expressions
#       (above) and the class name/comment.
class maya_mental(parser.parser):
   'Maya To Mental Ray plugin'
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
      else:
         match = re_percent.search( data)
         if match != None:
            needcalc = True

      if not needcalc: return

      match = re_number.search( match.group(0))
      if match == None: return

      try: number = int( match.group(0))
      except: needcalc = False

      if not needcalc: return
      
      if frame:
         if not self.firstframe: self.frame += 1
         self.firstframe = False
      else: self.percentframe = number
      
      self.calculate()
