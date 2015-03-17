# -*- coding: utf-8 -*-

import os
import sys
from . import addon_prefs


def load_module(module):
	try:
		loaded_module = __import__(module, globals(), locals(), [])
	except ImportError as err:
		msg = 'Unable to import %s Python module: %s' % (module, err)
		print(msg)
		self.report({'ERROR'}, msg)
		return {'CANCELLED'}
	return loaded_module


def check_cgru_env(context):
	addon_prefs = context.user_preferences.addons[__package__].preferences

	if 'CGRU_LOCATION' not in os.environ:
		os.environ['CGRU_LOCATION'] = addon_prefs.cgru_location
	else:
		addon_prefs.cgru_location = os.environ['CGRU_LOCATION']

	cgrupython = os.getenv('CGRU_PYTHON')
	if not cgrupython:
		if not addon_prefs.cgru_location:
			if sys.platform.find('win'):
				cgrupython = r'C:\cgru\lib\python'
			else:
				cgrupython = r'/opt/cgru/lib/python'
		else:
			cgrupython = os.path.join(addon_prefs.cgru_location, 'lib', 'python')

	if cgrupython not in sys.path:
		sys.path.append(cgrupython)

	# Check and add Afanasy module in system path:
	afpython = os.getenv('AF_PYTHON')
	if not afpython:
		if not addon_prefs.cgru_location:
			if sys.platform.find('win'):
				afpython = r'C:\cgru\afanasy\python'
			else:
				afpython = r'/opt/cgru/afanasy/python'
		else:
			afpython = os.path.join(addon_prefs.cgru_location, 'afanasy', 'python')

	if afpython not in sys.path:
		sys.path.append(afpython)


def get_movie_codecs(self, context):
	#check_cgru_env(context)
	addon_prefs = context.user_preferences.addons[__package__].preferences
	codecs_path = os.path.join(addon_prefs.cgru_location,
		'utilities','moviemaker','codecs')
	codecs = []
	try:
		codecs_files = os.listdir(codecs_path)
		for file in codecs_files:
			if '.ffmpeg' in file or '.mencoder' in file:
				codec_name = os.path.splitext(file)[0]
				codecs.append((codec_name,codec_name,''))
	except:
		pass

	return codecs

