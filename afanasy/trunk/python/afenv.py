#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
import socket
import xml.parsers.expat

class Env:
   def __init__( self, Verbose = False):
      self.verbose = Verbose
      self.valid = False
      afroot = os.getenv('AF_ROOT')
      if afroot == None:
         print 'Error: AF_ROOT is not defined.'
         return
      home = os.getenv('HOME', os.getenv('HOMEPATH'))
      if home == None: home = 'None'
      username = os.getenv('AF_USERNAME', os.getenv('USER', os.getenv('USERNAME')))
      if username == None: username = 'None'

      # cut DOMAIN from username:
      dpos = username.rfind('/')
      if dpos == -1: dpos = username.rfind('\\')
      if dpos != -1: username = username[dpos+1:]

      hostname = os.getenv('AF_HOSTNAME', socket.gethostname())
      if self.verbose:
         print 'Afanasy root = "%s"' % afroot
         print 'Host name = "%s"' % hostname
         print 'User name = "%s"' % username
         print 'User home = "%s"' % home
      self.Vars = {'afroot':afroot, 'home':home, 'username':username, 'hostname':hostname}
      self.element = ''
      self.element_hasdata = False
      filenames = [afroot+'/config_default.xml', afroot+'/config.xml', home+'/.afanasy/config.xml']
      for filename in filenames:
         if self.verbose: print 'Trying to open %s' % filename
         if os.path.isfile( filename):
            file = open( filename, 'r')
            filedata = file.read()
            file.close()
            parser = xml.parsers.expat.ParserCreate()
            parser.StartElementHandler    = self.start_element
            parser.EndElementHandler      = self.end_element
            parser.CharacterDataHandler   = self.char_data
            if self.verbose: print 'Parsing %s' % filename
            parser.Parse( filedata)
      self.valid = True
      if self.verbose: print self.Vars


   def start_element( self, name, attrs ):
      self.element_hasdata = False
      if name != 'afanasy': self.element = name
      else: self.element = ''

   def end_element( self, name ):
      if self.element == '': return
      if self.element_hasdata == False: self.Vars[self.element] = ''
      if self.verbose: print '\t' + self.element + ' = "%s"' % self.Vars[self.element]
      self.element = ''

   def char_data( self, data ):
      if self.element == '': return
      self.Vars[self.element] = str(data)
      self.element_hasdata = True
