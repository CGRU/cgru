from parsers import parser

FRAME = 'FRAME: '
PERCENT = 'PROGRESS: '
PERCENT_len = len(PERCENT)

class mayatomr(parser.parser):
   'Simple generic parser (mayatomr)'
   def __init__( self, frames = 1):
      parser.parser.__init__( self, frames)
      self.firstframe = True

   def do( self, data):
      #print 'parsing generic:'
      #print data
      needcalc = False
      if data.find(FRAME) > -1:
         if self.firstframe:
            self.firstframe = False
         else:
            self.frame += 1
            needcalc = True
      percent_pos = data.find(PERCENT)
      if percent_pos > -1:
         ppos = data.find('%')
         if ppos > -1:
            needcalc = True
            self.percentframe = int(data[percent_pos+PERCENT_len:ppos])
      if( needcalc ): self.calculate()
