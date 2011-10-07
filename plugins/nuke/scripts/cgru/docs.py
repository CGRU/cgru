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
   cgru = getenv('CGRU_LOCATION')
   if cgru is None: return
   path = getenv('NUKE_CGRU_PATH')
   if path is None: return
   path = path.replace( cgru, '')
   path = os.path.join( path, 'doc')
   path = os.path.join( path, 'index.html')
   cgrudocs.show( path)

def docsAfanasy():
   cgru = getenv('CGRU_LOCATION')
   if cgru is None: return
   path = getenv('NUKE_AF_PATH')
   if path is None: return 
   path = path.replace( cgru, '')
   path = os.path.join( path, 'doc')
   path = os.path.join( path, 'afanasy.html')
   cgrudocs.show( path)
