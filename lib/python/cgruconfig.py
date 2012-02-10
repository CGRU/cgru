import os
import socket
import stat
import sys
import time
import xml.parsers.expat

import cgruutils

VARS = dict()

def checkConfigFile( path, name):
   status = True
   if not os.path.isfile( path):
      try:
         cfile = open( path, 'w')
      except:
         print(str(sys.exc_info()[1]))
         status = False
      if status:
         cfile.write('<!-- Created by CGRU Keeper at ' + time.ctime() + ' -->\n')
         cfile.write('<'  + name + '>\n')
         cfile.write('</' + name + '>\n')
         cfile.close()
   if status:
      try:
         os.chmod( path, stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)
      except:
         print(str(sys.exc_info()[1]))
   return status


class Config:
   def __init__( self, variables = VARS, configfiles = None, Verbose = False):
      self.verbose = Verbose
      self.Vars = variables

      if configfiles is None:
         self.Vars['hostname'] = socket.gethostname().lower()

         cgrulocation =  os.getenv('CGRU_LOCATION')
         if cgrulocation is None or cgrulocation == '': return

         # Definitions which always must preset:
         self.Vars['CGRU_LOCATION'] = cgrulocation
         self.Vars['CGRU_VERSION'] = os.getenv('CGRU_VERSION','')
         self.Vars['CGRU_PYTHONEXE'] = os.getenv('CGRU_PYTHONEXE','python')
         self.Vars['CGRU_UPDATE_CMD'] = os.getenv('CGRU_UPDATE_CMD')

         self.Vars['company'] = 'CGRU'
         self.Vars['menu_path'] = None
         self.Vars['tray_icon'] = None
         self.Vars['icons_path'] = None
         if sys.platform.find('win') == 0:
            self.Vars['editor'] = 'notepad "%s"'
         else:
            self.Vars['editor'] = 'xterm -e vi "%s"'

         self.Vars['config_file'] = os.path.join( cgrulocation, 'config.xml')
         home = os.getenv('HOME', os.getenv('HOMEPATH'))
         self.Vars['HOME'] = home
         self.Vars['HOME_CGRU'] = os.path.join( home, '.cgru')
         self.Vars['HOME_AFANASY'] = os.path.join( home, '.afanasy')
         self.Vars['config_file_home'] = os.path.join( self.Vars['HOME_CGRU'], 'config.xml')
         self.Vars['config_afanasy'] = os.path.join( self.Vars['HOME_AFANASY'], 'config.xml')
         if sys.platform.find('win') == 0 or os.geteuid() != 0:
            cgruutils.createFolder( self.Vars['HOME_CGRU']    )
            cgruutils.createFolder( self.Vars['HOME_AFANASY'] )
            # Create cgru home config file if not preset
            checkConfigFile( self.Vars['config_file_home'], 'cgru')
            # Create afanasy home config file if not preset
            checkConfigFile( self.Vars['config_afanasy'], 'afanasy')

         configfiles = []
         configfiles.append( os.path.join( cgrulocation, 'config_default.xml'))
         configfiles.append( self.Vars['config_file'])
         configfiles.append( self.Vars['config_file_home'])
         configfiles.append( self.Vars['config_afanasy'])

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
      if name != 'cgru' and name != 'afanasy': self.element = name
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

def writeVars( variables, configfile = VARS['config_file_home']):
   configname = 'cgru'
   if configfile == VARS['config_afanasy']: configname = 'afanasy'
   file = open( configfile,'r')
   lines = file.readlines()
   file.close()
   for var in variables:
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
            if line.find('<%s>' % configname) != -1: continue
            lines.insert( num, toinsert)
            break
   file = open( configfile,'w')
   for line in lines: file.write( line)
   file.close()
