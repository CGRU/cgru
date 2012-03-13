import os
import webbrowser

import cgruconfig

def browse( path):
   webbrowser.open( path)

def show( path = None):
   cgrulocation =  os.getenv('CGRU_LOCATION')
   if cgrulocation is None or cgrulocation == '':
      print 'Variable "CGRU_LOCATION" is not set.'
      return

   filenames = []
   filenames.append( os.path.join( cgrulocation, 'config_default.xml'))
   filenames.append( os.path.join( cgrulocation, 'config.xml'))

   VARS = dict()
   cgruconfig.Config( VARS, filenames)
   key = 'docshost'
   location = None
   if key in VARS:
      key = VARS[key]
      if key is not None and key != '':
         location = 'http://%s' % key
         if path is not None:
            if path[0] == '/' or path[0] == '\\':
               location += path
            else:
               location = '%s/%s' % (location, path)
   if location is None:
      location = cgrulocation
      print 'location = "%s"' % location
      if path is None or path == '':
         path = 'index.html'
      if path[0] == '/' or path[0] == '\\':
         location += path
      else:
         location = os.path.join( location, path)
      print 'location = "%s"' % location

   browse( location)
