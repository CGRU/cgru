import os
import sys
import time
import xml.parsers.expat
import socket

VARS = dict()

class Config:
   def __init__( self, variables = VARS, configfiles = None, Verbose = False):
      self.verbose = Verbose
      self.Vars = variables

      if configfiles is None:
         self.Vars['hostname'] = socket.gethostname().lower()

         cgrulocation =  os.getenv('CGRU_LOCATION')
         if cgrulocation is None or cgrulocation == '': return

         self.Vars['CGRU_LOCATION'] = cgrulocation

         self.Vars['CONFIGFILE'] = os.path.join( cgrulocation, 'config.xml')
         home = os.getenv('HOME', os.getenv('HOMEPATH'))
         self.Vars['HOME'] = home
         self.Vars['HOME_CGRU'] = os.path.join( home, '.cgru')
         if not os.path.isdir( self.Vars['HOME_CGRU']):
            os.mkdir( self.Vars['HOME_CGRU'])
         self.Vars['HOME_CONFIGFILE'] = os.path.join( self.Vars['HOME_CGRU'], 'config.xml')
         # Create home config file if not preset
         if not os.path.isfile( self.Vars['HOME_CONFIGFILE']):
            cfile = open( self.Vars['HOME_CONFIGFILE'], 'w')
            cfile.write('<!-- Created at ' + time.ctime() + ' -->\n')
            cfile.write('<cgru>\n')
            cfile.write('</cgru>\n')
            cfile.close()

         configfiles = []
         configfiles.append( os.path.join( cgrulocation, 'config_default.xml'))
         configfiles.append( self.Vars['CONFIGFILE'])
         configfiles.append( self.Vars['HOME_CONFIGFILE'])

      for filename in configfiles:
         if self.verbose: print('Trying to open %s' % filename)
         if os.path.isfile( filename):
            file = open( filename, 'r')
            filedata = file.read()
            file.close()
            parser = xml.parsers.expat.ParserCreate()
            parser.StartElementHandler    = self.parser_start_element
            parser.EndElementHandler      = self.parser_end_element
            parser.CharacterDataHandler   = self.parser_char_data
            if self.verbose: print('Parsing %s' % filename)
            try:
               parser.Parse( filedata)
            except xml.parsers.expat.ExpatError as err:
               print("Error:", xml.parsers.expat.errors.messages[err.code], ': line', err.lineno, ', column', err.offset)

   def parser_start_element( self, name, attrs ):
      self.element_hasdata = False
      if name != 'cgru': self.element = name
      else: self.element = ''

   def parser_end_element( self, name ):
      if self.element == '': return
      if self.element_hasdata == False: self.Vars[self.element] = ''
      if self.verbose: print('\t' + self.element + ' = "%s"' % self.Vars[self.element])
      self.element = ''

   def parser_char_data( self, data ):
      if self.element == '': return
      self.Vars[self.element] = str(data)
      self.element_hasdata = True

Config()

def writeVars( variables):
   file = open(VARS['HOME_CONFIGFILE'],'r')
   lines = file.readlines()
   file.close()
   for var in variables:
#      if var not in VARS: continue
      tofind = '<'+var+'>'
      toinsert = '   <'+var+'>' + VARS[var] + '</'+var+'>   <!-- Modified at '+time.ctime()+' -->\n'
      founded = False
      num = -1
      for line in lines:
         num += 1
         if line.find( tofind) == -1: continue
         founded = True
         lines[num] = toinsert
         break
      if not founded:
         num = 1
         for line in lines:
            num += 1
            if line.find('<cgru>') != -1: continue
            lines.insert( num, toinsert)
            break
   file = open(VARS['HOME_CONFIGFILE'],'w')
   for line in lines: file.write( line)
   file.close()
