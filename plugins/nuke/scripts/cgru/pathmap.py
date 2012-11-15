import os, sys
import shutil
import socket

from cgrupathmap import PathMap

import nuke

SERVER_PATHS_SUFFIX = 'SERVERPATHS'
CLIENT_PATHS_SUFFIX = 'CLIENTPATHS'
SearchStrings = ['file ','font ', 'project_directory ']

def pmSaveToServer():
   print 'Saving scene to server paths...'
   pm = PathMap( UnixSeparators = True, Verbose = True)
   nuke.scriptSave()

   # Get server scene name:
   scenename = nuke.root().name()
   scenename_server = scenename + '.' + SERVER_PATHS_SUFFIX + '.nk'

   # Map paths from client to server:
   error_msg = ''
   if pm.initialized:
      pm.toServerFile( scenename, scenename_server, SearchStrings, Verbose = True)
   else:
      print 'No paths map preset. Just copying scene to:'
      print scenename_server
      try:
         shutil.copy( scenename, scenename_server)
      except:
         error_msg = str(sys.exc_info()[1])
         print 'File copied with error:'
         print error_msg

   if error_msg != '': nuke.message('Server scene copy error:\n' + error_msg)

def pmOpenFromServer():
   print 'Opening scene with server paths...'
   pm = PathMap( UnixSeparators = True, Verbose = True)

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
   error_msg = ''
   if pm.initialized:
      pm.toClientFile( scenename_server, scenename_client, SearchStrings, Verbose = True)
   else:
      print 'No paths map preset. Just copying scene to:'
      print scenename_client
      try:
         shutil.copy( scenename_server, scenename_client)
      except:
         error_msg = str(sys.exc_info()[1])
         print 'File copied with error:'
         print error_msg
         error_msg = '\n' + error_msg

   # Check if new scene exists:
   if not os.path.isfile(scenename_client):
      nuke.message('Client scene was not created.' + error_msg)
      return

   # Open client scene:
   nuke.scriptOpen( scenename_client)
