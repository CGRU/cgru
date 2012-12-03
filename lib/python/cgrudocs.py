import os
import webbrowser

import cgruconfig

def browse( path):
	webbrowser.open( path)

def show( path = None):

	docs_host = 'docs_host'

	if path is not None:
		if path[:-5] != '.html':
			path += '.html'
		if path[0] != '#':
			path = '#' + path

	if docs_host in cgruconfig.VARS:
		docs_host = cgruconfig.VARS[docs_host]
		if docs_host is not None and docs_host != '':
			location = 'http://%s' % docs_host
			if path is not None:
				if path[0] == '/' or path[0] == '\\':
					location += path
				else:
					location = '%s/%s' % (location, path)
	else:
		cgrulocation =  os.getenv('CGRU_LOCATION')
		if cgrulocation is None or cgrulocation == '':
			print('No "%s" variable and "CGRU_LOCATION" is set.' % docs_host)
			return
		location = cgrulocation
		if path is None or path == '':
			path = 'index.html'
		if path[0] == '/' or path[0] == '\\':
			location += path
		else:
			location = os.path.join( location, path)

	browse( location)
