import os
import shutil

from afpathmap import PathMap

import nuke

def saveToServer():
   print 'Saving scene to server paths...'
   pm = PathMap( os.environ['AF_ROOT'], UnixSeparators = True, Verbose = True)
   nuke.scriptSave()
   scenename = nuke.root().name()
   scenename_server = scenename + '.SERVERPATHS.nk'
   if pm.initialized:
      pm.toServerFile( scenename, scenename_server, Verbose = True)
   else:
      print 'No paths map preset. Just copying scene to:'
      print scenename_server
      shutil.copy( scenename, scenename_server)
