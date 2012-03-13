import os
import sys
import xml.parsers.expat

class Config:
   def __init__( self, VARS, configfiles, Verbose = False):
      self.verbose = Verbose
      self.valid = False
      self.Vars = VARS

      cgrulocation =  os.getenv('CGRU_LOCATION')
      if cgrulocation is not None:
         if cgrulocation != '':
            self.Vars['CGRU_LOCATION'] = cgrulocation

      for filename in configfiles:
         if self.verbose: print 'Trying to open %s' % filename
         if os.path.isfile( filename):
            file = open( filename, 'r')
            filedata = file.read()
            file.close()
            parser = xml.parsers.expat.ParserCreate()
            parser.StartElementHandler    = self.parser_start_element
            parser.EndElementHandler      = self.parser_end_element
            parser.CharacterDataHandler   = self.parser_char_data
            if self.verbose: print 'Parsing %s' % filename
            parser.Parse( filedata)

   def parser_start_element( self, name, attrs ):
      self.element_hasdata = False
      if name != 'cgru': self.element = name
      else: self.element = ''

   def parser_end_element( self, name ):
      if self.element == '': return
      if self.element_hasdata == False: self.Vars[self.element] = ''
      if self.verbose: print '\t' + self.element + ' = "%s"' % self.Vars[self.element]
      self.element = ''

   def parser_char_data( self, data ):
      if self.element == '': return
      self.Vars[self.element] = str(data)
      self.element_hasdata = True
