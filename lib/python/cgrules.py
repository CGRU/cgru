# -*- coding: utf-8 -*-

import cgruconfig
import cgruutils

def getShotURL( i_path, i_verbose = False):
	"""Missing DocString

	:param i_file:
	:param i_verbose:
	:return:
	"""

	path = i_path.replace('\\','/')

	if i_verbose:
		print('Projects:')
		print(cgruconfig.VARS['rules_projects'])

	for prj in cgruconfig.VARS['rules_projects']:
		if path.find( prj) == 0:
			path = path.replace(prj,'')
			break

	max_depth = 4 # PROJECT/SCENES/SCENE/SHOT

	path = path.split('/')

	if len(path) > max_depth:
		path = path[0:4]

	path = '/'.join( path)

	if i_verbose: print('Path = %s' % path)

	path = cgruconfig.VARS['rules_url'] + path

	if i_verbose: print('URL = %s' % path)

	return path

def openShot( i_path, i_verbose = False):
	cgruutils.webbrowse( getShotURL( i_path, i_verbose))

