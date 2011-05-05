import os
import shutil
import socket

from afpathmap import PathMap

import nuke

SERVER_PATHS_SUFFIX = 'SERVERPATHS'
CLIENT_PATHS_SUFFIX = 'CLIENTPATHS'

def pmSaveToServer():
   print 'Saving scene to server paths...'
   pm = PathMap( os.environ['AF_ROOT'], UnixSeparators = True, Verbose = True)
   nuke.scriptSave()

   # Get server scene name:
   scenename = nuke.root().name()
   scenename_server = scenename + '.' + SERVER_PATHS_SUFFIX + '.nk'

   # Map paths from client to server:
   if pm.initialized:
      pm.toServerFile( scenename, scenename_server, Verbose = True)
   else:
      print 'No paths map preset. Just copying scene to:'
      print scenename_server
      shutil.copy( scenename, scenename_server)

def pmOpenFromServer():
   print 'Opening scene with server paths...'
   pm = PathMap( os.environ['AF_ROOT'], UnixSeparators = True, Verbose = True)

   # Get server scene name:
   scenename_server = nuke.getFilename('Select a scene with server paths','*.nk')
   if scenename_server is None: return

   # Get client scene name
   scenename_client = scenename_server
   clientname = CLIENT_PATHS_SUFFIX
   clientname += '-' + socket.gethostname()
   if scenename_client.find( SERVER_PATHS_SUFFIX) != -1:
      scenename_client = scenename_client.replace( SERVER_PATHS_SUFFIX, clientname)
   else:
      scenename_client += '.' + clientname + '.nk'

   # Map paths from server to client:
   if pm.initialized:
      pm.toClientFile( scenename_server, scenename_client, Verbose = True)
   else:
      print 'No paths map preset. Just copying scene to:'
      print scenename_client
      shutil.copy( scenename_server, scenename_client)

   # Open client scene:
   nuke.scriptOpen( scenename_client)
