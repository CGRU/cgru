# -*- coding: utf-8 -*-

import os
import sys
import bpy

CGRU_NOT_FOUND = 'NOT FOUND'


def get_cgru_version(path):
    try:
        with open(os.path.join(path, 'version.txt'), 'r') as f:
            version = f.read().strip()
        return version
    except:
        return CGRU_NOT_FOUND


def add_cgru_module_to_syspath(path):
    cgrumodule = os.path.join(path, 'lib', 'python')
    if cgrumodule not in sys.path:
        sys.path.append(cgrumodule)
    afmodule = os.path.join(path, 'afanasy', 'python')
    if afmodule not in sys.path:
        sys.path.append(afmodule)

    prefs = bpy.context.user_preferences.addons[__package__].preferences
    if "CGRU_LOCATION" not in os.environ:
        os.environ["CGRU_LOCATION"] = prefs.cgru_location


def get_movie_codecs(self, context):
    addon_prefs = context.user_preferences.addons[__package__].preferences
    codecs_path = os.path.join(
        addon_prefs.cgru_location,
        'utilities',
        'moviemaker',
        'codecs')
    codecs = []
    try:
        codecs_files = os.listdir(codecs_path)
        for file in codecs_files:
            if '.ffmpeg' in file or '.mencoder' in file:
                codec_name = os.path.splitext(file)[0]
                codecs.append((codec_name, codec_name, ''))
    except:
        pass

    return codecs
