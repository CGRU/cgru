import json
import os
import traceback

import rulib

def getCommentsFilePath(i_path = None):
    return rulib.functions.getRuFilePath('comments.json', i_path)

def getCommentsData(i_path = None, out = None):
    data = rulib.functions.readObj(getCommentsFilePath(i_path), out)
    if data is None:
        return None
    if 'comments' in data:
        return data['comments']
    error = 'Comments data not found'
    if out is not None: out['error'] = error
    else: print(error)
    return  None

def saveCommentsData(i_path, i_data, out=None):
    path = getCommentsFilePath(i_path)
    rufolder = os.path.dirname(path)
    if not os.path.isdir(rufolder):
        try:
            os.makedirs(rufolder)
        except PermissionError:
            err = 'Permissions denied to make folder for comments: ' + rufolder
            if out is not None: out['error'] = err
            else: print(err)
            return
        except:
            err = 'Unable to create folder for comments: ' + rufolder
            err += '\n%s' % traceback.format_exc()
            if out is not None: out['error'] = err
            else: print(err)
            return
    rulib.functions.writeObj(path, {'comments':i_data}, out)

class Comments:

    def __init__(self, uid=None, path=None):

        if uid is None:
            uid = rulib.functions.getCurUser()

        self.uid = uid
        self.path = rulib.functions.getRootPath(path)

        self.data = getCommentsData(self.path)
        if self.data is None:
            self.data = dict()

    def __repr__(self):
        return json.dumps(self.data)

    def __str__(self):
        return json.dumps({"comments":self.data})

    def add(self, ctype=None, text=None, tags=None, duration=None, color=None, deleted=False, uploads=None, out=None, key=None):

        cdata = dict()
        ctime = rulib.functions.getCurMSeconds()
        if key is None:
            key = "%d_%s" % (ctime, self.uid)
            cdata['cuser'] = self.uid
            cdata['ctime'] = ctime
            cdata['key'] = key
        else:
            if not key in self.data:
                out['error'] = 'No comments with key=%s' % key
                return None
            cdata = self.data[key]
            cdata['muser'] = self.uid
            cdata['mtime'] = ctime

        if ctype is not None:
            cdata['type'] = ctype
        if text is not None:
            cdata['text'] = text
        if tags is not None and type(tags) is list:
            cdata['tags'] = tags
        if duration is not None:
            cdata['duration'] = duration
        if color is not None and type(color) is list:
            if len(color) == 3:
                cdata['color'] = color
            else:
                if 'color' in cdata:
                    del cdata['color']
        if uploads:
            cdata['uploads'] = uploads
        if deleted:
            cdata['deleted'] = True

        self.data[key] = cdata

        return cdata

    def save(self, out=dict()):
        saveCommentsData(self.path, self.data, out)
        out['comments'] = self.data

