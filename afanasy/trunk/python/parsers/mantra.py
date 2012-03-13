import parser

PERCENT = 'ALF_PROGRESS '
PERCENT_len = len(PERCENT)

class mantra(parser.parser):
   'Houdini mantra with "Alfred Style Progress"'
   def __init__( self, frames = 1):
      parser.parser.__init__( self, frames)

   def do( self, data):
      percent_pos = data.find(PERCENT)
      if percent_pos > -1:
         ppos = data.find('%')
         if ppos > -1:
            percent = int(data[percent_pos+PERCENT_len:ppos])
            if percent >= -1 and percent <= 100:
               self.percentframe = percent
               self.percent = percent
               return True
      return False
