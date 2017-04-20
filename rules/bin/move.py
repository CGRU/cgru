#!/usr/bin/env python

import json
import os
import shutil
import sys

os.umask(0000)

src = sys.argv[1]
dst = sys.argv[2]

Out = dict()
Out['src'] = src
Out['dst'] = dst


def errExit(i_msg):
	Out['error'] = i_msg
	Out['status'] = 'error'
	print(json.dumps({'move': Out}, indent=4))
	sys.exit(1)

def checkPath( i_path):
	if not os.access( i_path, os.F_OK):
		errExit('Not exists: ' + i_path)
	if not os.access( i_path, os.R_OK):
		errExit('Permissions denied: ' + i_path)
	if not os.access( i_path, os.W_OK):
		errExit('Can`t modify: ' + i_path)


checkPath( src)
checkPath( os.path.dirname( src))
checkPath( os.path.dirname( dst))


try:
	shutil.move( src, dst)
except:
	errExit(str(sys.exc_info()[1]))


Out['status'] = 'success'

print(json.dumps({'move': Out}, indent=4))

