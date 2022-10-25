import os
import time
import traceback

from rulib import functions

def search(i_args, o_out, i_path, i_depth):

    i_depth += 1
    if i_depth > i_args['depth']:
        return

    if not os.path.isdir(i_path):
        return

    try:
        listdir = os.listdir(i_path)
    except:
        return

    for entry in listdir:
        if entry ==  '.': continue
        if entry == '..': continue

        path = os.path.join(i_path, entry)
        if not os.path.isdir(path):
            continue

        found = False
        rufolder = os.path.join(path, i_args['rufolder'])
        if os.path.isdir(rufolder):
            found = True

        if found and 'status' in i_args:
            found = False
            rufile = os.path.join(rufolder, 'status.json')
            if os.path.isfile(rufile):
                obj = functions.readObj(rufile)
                if obj and searchStatus(i_args['status'], obj):
                        found = True

        if found and 'body' in i_args:
            found = False
            rufile = os.path.join(rufolder, 'body.html')
            if os.path.isfile(rufile):
                data = functions.fileRead(rufile)
                if data and data.find(i_args['body']) != -1:
                    found = True

        if found and 'comment' in i_args:
            found = False
            rufile = os.path.join(rufolder, 'comments.json')
            if os.path.isfile(rufile):
                obj = functions.readObj(rufile)
                if obj and searchComment(i_args['comment'], obj):
                    found = True

        if found:
            o_out['result'].append(path)

        if i_depth < i_args['depth']:
            search(i_args, o_out, path, i_depth)


def searchStatus(i_args, i_obj):
    if not isinstance(i_obj, dict):
        return False
    if not 'status' in i_obj:
        return False

    found = True

    if found and 'ann' in i_args:
        found = False
        if 'annotation' in i_obj['status']:
            if i_obj['status']['annotation'].find(i_args['ann']) != -1:
                found = True

    if found and 'artists' in i_args:
        found = False
        if 'artists' in i_obj['status'] and len(i_obj['status']['artists']):
            for artist in i_args['artists']:
                if artist in i_obj['status']['artists']:
                    found = True
        elif '_null_' in i_args['artists']:
            found = True

    if found and 'tags' in i_args:
        found = False
        if 'tags' in i_obj['status']:
            for tag in i_args['tags']:
                if tag in i_obj['status']['tags']:
                    found = True

    if found and 'percent' in i_args:
        found = False
        if 'progress' in i_obj['status']:
            if (i_obj['status']['progress'] >= i_args['percent'][0]) and (i_obj['status']['progress'] <= i_args['percent'][1]):
                found = True

    for parm in ['finish', 'statmod', 'bodymod']:
        if found and parm in i_args:
            found = False

            val = parm
            if parm == 'statmod':
                val = 'mtime'
            elif parm == 'bodymod':
                val = 'body'

            if val in i_obj['status']:
                if val == 'body':
                    val = i_obj['status']['body']['mtime']
                else:
                    val = i_obj['status'][val]

                val = (val - int(time.time())) / (60 * 60 * 24)

                if (val >= i_args[parm][0]) and (val <= i_args[parm][1]):
                    found = True

    return found


def searchComment(i_comment, i_obj):
    if not isinstance(i_obj, dict):
        return False
    if not 'comments' in i_obj:
        return False

    for comment in i_obj['comments']:
        if 'text' in comment:
            if comment['text'].find(i_comment) != -1:
                return True

    return False

