#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import shutil

store = ['FindPostgreSQL.cmake', '.gitignore', '.svn', 'build.sh',
		 'build_ostype.sh', 'override.sh', 'override.cmd', 'clear.py',
		 'clear.cmd', 'CMakeLists.txt', 'win_build_msvc_pyaf.cmd',
		 'win_build_msvc.cmd']
projects = ['libafanasy', 'libafqt', 'libafsql', 'cmd', 'server', 'render',
			'watch', 'fermer', 'qml', 'service']
tests_generated = ['CMakeFiles', 'Testing', 'Makefile', 'cmake_install.cmake',
				   'CTestTestfile.cmake', 'CMakeCache.txt', 'DartConfiguration.tcl']
tests_build_ext = ['.obj', '.o', '.exe', '.pdb', '.ilk', '.a', '.lib', '.exp',
				   '.manifest', '.dll', '.so', '.dylib']

DEBUG = True
DEBUG = False


def delete(item):
	if os.path.isdir(item):
		print('Deleting directory "%s"' % item)
		if not DEBUG:
			shutil.rmtree(item)
	else:
		print('Deleting file "%s"' % item)
		if not DEBUG:
			os.remove(item)


def clean_tests():
	tests = 'tests'
	if not os.path.isdir(tests):
		return

	for item in os.listdir(tests):
		path = os.path.join(tests, item)
		if item in tests_generated:
			delete(path)
			continue

		if not os.path.isfile(path):
			continue

		ext = os.path.splitext(item)[1].lower()
		if ext in tests_build_ext:
			delete(path)
			continue
		if ( ext == '') and os.access(path, os.X_OK):
			delete(path)


for item in os.listdir('.'):
	if item == 'tests':
		clean_tests()
		continue
	if item in store:
		continue
	if item in projects:
		project = item
		for item in os.listdir(project):
			if item in store:
				continue
			delete(os.path.join(project, item))
		continue
	delete(item)
