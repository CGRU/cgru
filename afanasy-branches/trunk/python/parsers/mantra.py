from parsers import parser

import re

PERCENT = 'ALF_PROGRESS '
PERCENT_len = len(PERCENT)

Warnings = ['Unable to access file','Unable to load texture']
Errors = ['No licenses could be found to run this application','Please check for a valid license server host','Failed to create file']
ErrorsRE = [re.compile(r'Error loading geometry .* from stdin')]

class mantra(parser.parser):
   'Houdini mantra with "Alfred Style Progress"'
   def __init__( self, frames = 1):
      parser.parser.__init__( self, frames)

   def do( self, data):

      for warning in Warnings:
         if data.find(warning) != -1:
            self.warning = True
            break

      for error in Errors:
         if data.find(error) != -1:
            self.error = True
            break

      for errorRE in ErrorsRE:
         if errorRE.search( data) is not None:
            self.error = True
            break

      percent_pos = data.find(PERCENT)
      if percent_pos > -1:
         ppos = data.find('%')
         if ppos > -1:
            try:
               percent = int(data[percent_pos+PERCENT_len:ppos])
            except:
               pass
            if percent >= -1 and percent <= 100:
               self.percentframe = percent
               self.calculate()
