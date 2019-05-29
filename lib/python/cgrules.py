# -*- coding: utf-8 -*-

import json
import os
import traceback

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


def getRules(i_path = None, i_verbose = False):
    rules = dict()
    for afile in getRuFiles(i_path):
        if i_verbose:
            print('File: "%s"' % afile)

        try:
            fobj = open(afile, 'r')
        except:
            print(traceback.format_exc())
            continue

        obj = None
        try:
            obj = json.load(fobj)
        except:
            print(traceback.format_exc())
            continue

        fobj.close()

        if obj:
            mergeObjects(rules, obj)

    return rules


def getRuFiles(i_path = None, i_ruFolder = '.rules'):
    if i_path is None: i_path = os.getcwd()
    files = []

    # Get all upper folders:
    path = i_path
    prev = None
    folders = []
    while prev != path:
        folders.append(path)
        prev = path
        path = os.path.dirname(path)

    # Iterate folders from top to find files:
    for folder in reversed(folders):
        ruFolder = os.path.join(folder, i_ruFolder)
        if not os.path.isdir(ruFolder):
            continue
        ruFiles = []
        for afile in os.listdir(ruFolder):
            if afile.find('rules') != 0:
                continue
            if afile.find('.json') == -1:
                continue
            ruFiles.append(os.path.join(ruFolder, afile))
        ruFiles.sort()
        files += ruFiles

    return files


def mergeObjects(o_obj, i_obj):
    for key in i_obj:
        if len(key) == 0: continue
        if key[0] in ' -_#!': continue
        if isinstance(i_obj[key], dict):
            if not key in o_obj or not isinstance(o_obj[key], dict):
                o_obj[key] = dict()
            mergeObjects(o_obj[key], i_obj[key])
        else:
            o_obj[key] = i_obj[key]
