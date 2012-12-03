import nuke

import os
import webbrowser

import cgrudocs

def browse( path):
   webbrowser.open( path)

def getenv( name):
   value = os.getenv( name)
   if value is None:
      nuke.message('Variable "%s" is not set.' % name)
      return None
   if value == '':
      nuke.message('Variable "%s" is empty.' % name)
      return None
   return value

def docsCGRU():
   cgrudocs.show()

def docsNuke():
   cgrudocs.show('nuke')

