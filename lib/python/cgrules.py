# -*- coding: utf-8 -*-

import cgruconfig
import cgruutils


def hasRULES():
	return 'rules_url' in cgruconfig.VARS

def toRulesURL( i_path, i_verbose = False):
	"""
		Prepend Rules URL to path
	"""
	if not 'rules_url' in cgruconfig.VARS:
		print('Rules URL is not defined.')
		return None

	return cgruconfig.VARS['rules_url'] + i_path


def toProjectRoot( i_path, i_verbose = False):
	"""
		Transfer path relative to projects root
	"""
	if not 'projects_root' in cgruconfig.VARS:
		print('Projects root is not set.')
		return None

	if i_verbose:
		print('Projects root:')
		print(cgruconfig.VARS['projects_root'])

	path = i_path.replace('\\','/')

	for prj in cgruconfig.VARS['projects_root']:
		if path.find( prj) == 0:
			path = path.replace(prj,'')
			break

	return path


def getPathURL( i_path, i_verbose = False):
	"""
		Get path in Rules
	"""
	path = i_path.replace('\\','/')

	path = toProjectRoot( path, i_verbose)
	if path is None:
		return None

	return toRulesURL( path, i_verbose)


def openPath( i_path, i_verbose = False):
	"""
		Open path in Rules
	"""
	url = getPathURL( i_path, i_verbose)
	if url:
		if i_verbose:
			print('URL = %s' % url)
		cgruutils.webbrowse( url)


def getShotURL( i_path, i_verbose = False):
	""" Get Rules Shot URL from given path to any file/folder in the shot
	:param i_path:
	:param i_verbose:
	:return:
	"""

	path = toProjectRoot( i_path)
	if path is None:
		return None

	max_depth = 4 # PROJECT/SCENES/SCENE/SHOT

	path = path.split('/')

	if len(path) > max_depth:
		path = path[0:4]

	path = '/'.join( path)

	return toRulesURL( path)


def openShot( i_path, i_verbose = False):
	"""
		Open Rules Shot URL from given path to any file/folder in the shot
	"""
	url = getShotURL( i_path, i_verbose)
	if url:
		if i_verbose:
			print('URL = %s' % url)
		cgruutils.webbrowse( url)

