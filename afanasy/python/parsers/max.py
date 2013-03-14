from parsers import parser

import re

re_frame   = re.compile(r'Frame [0-9]{1,} completed')
re_percent = re.compile(r'progr:[ ]{,}[0-9]{1,2}.*%[ ]{,}rendered')
re_number  = re.compile(r'[0-9]{1,}')

Errors = ['Error occured while rendering job.']

class max(parser.parser):
   '3ds Max'
   def __init__( self, frames = 1):
      parser.parser.__init__( self, frames)
      self.firstframe = True

   def do( self, data):

      for error in Errors:
         if data.find(error) != -1:
            self.error = True
            break

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
