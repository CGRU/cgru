import os, sys
import shutil
import socket

import cgrupathmap
import cgruconfig

import nuke

PathMap = cgrupathmap.PathMap( UnixSeparators = True)

PMaps = dict()
if 'platforms' in cgruconfig.VARS:
	for platform in cgruconfig.VARS['platforms']:
		if ('OS_'+platform) in cgruconfig.VARS:
			if 'pathsmap' in cgruconfig.VARS['OS_'+platform]:
				pm = cgrupathmap.PathMap( UnixSeparators = True)
				pm.init( cgruconfig.VARS['OS_'+platform]['pathsmap'])
				if pm.initialized:
					PMaps[platform] = pm

SearchStrings = ['file ','font ', 'project_directory ']

def pmFilenameFilter( filename):
#	print('FilenameFilter before:\n'+filename)
	# Transfer scene paths to server from all patforms:
	for key in PMaps:
		filename = PMaps[key].toServer( filename)
	# Transfer scene paths from server clinet native patform:
	if PathMap.initialized:
		filename = PMaps[key].toClient( filename)
#	print('FilenameFilter after:\n'+filename)
	return filename

if 'nuke_filenamefilter' in cgruconfig.VARS and cgruconfig.VARS['nuke_filenamefilter']:
	if 'platforms' in cgruconfig.VARS and 'pathsmap' in cgruconfig.VARS:
		nuke.addFilenameFilter( pmFilenameFilter)
		info = 'CGRU filenameFilter added:'
		for key in PMaps: info += ' '+key
		print( info)

def pmOpenTranslated():
	print('Opening scene with paths map...')

	# Get scene name:
	scenename_server = nuke.getFilename('Select a scene','*.nk')
	if scenename_server is None: return

	tmp_scenes = []
	last_scene = scenename_server
	# Transfer scene paths to server from all patforms:
	for key in PMaps:
		tmp_scenes.append( scenename_server+'.'+key)
		print('Transfering from "%s" to "%s"' % ( key, os.path.basename( tmp_scenes[-1])))
		PMaps[key].toServerFile( last_scene, tmp_scenes[-1], SearchStrings, Verbose = True)
		last_scene = tmp_scenes[-1]

	error_msg = ''
	# Transfer scene paths from server clinet native patform:
	scenename_client = scenename_server + '.' + socket.gethostname() + '.nk'
	if PathMap.initialized:
		print('Transfering from server to "%s"' %  os.path.basename( scenename_client))
		PathMap.toClientFile( last_scene, scenename_client, SearchStrings, Verbose = True)
	else:
		print('No paths map preset. Just copying scene to:')
		print( scenename_client)
		try:
			shutil.copy( last_scene, scenename_client)
		except:
			error_msg = str(sys.exc_info()[1])
			print('File copied with error:')
			print( error_msg)
			error_msg = '\n' + error_msg

	# Remove temp scenes:
	for scene in tmp_scenes:
		try:
			os.remove( scene)
		except:
			print( str(sys.exc_info()[1]))
			print('Error removing "%s"' % scene)

	# Check if new scene exists:
	if not os.path.isfile(scenename_client):
		nuke.message('Client scene was not created.' + error_msg)
		return

	# Open client scene:
	nuke.scriptOpen( scenename_client)

