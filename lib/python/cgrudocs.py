import os
import webbrowser

import cgruconfig
import cgruutils

def show(i_path=None):
	docs_host = 'docs_host'

	location = ''
	if docs_host in cgruconfig.VARS:
		docs_host = cgruconfig.VARS[docs_host]
		if docs_host is not None and docs_host != '':
			location = 'http://%s' % docs_host
			if i_path is not None:
				if i_path[0] == '/' or i_path[0] == '\\':
					location += i_path
				else:
					location = '%s/%s' % (location, i_path)
	else:
		cgrulocation = os.getenv('CGRU_LOCATION')
		if cgrulocation is None or cgrulocation == '':
			print('No "%s" variable and "CGRU_LOCATION" is set.' % docs_host)
			return
		location = cgrulocation
		if i_path is None or i_path == '':
			i_path = 'index.html'
		if i_path[0] == '/' or i_path[0] == '\\':
			location += i_path
		else:
			location = os.path.join(location, i_path)

	cgruutils.webbrowse(location)

def showSoftware( i_soft):
    return show('software/%s' % i_soft)

