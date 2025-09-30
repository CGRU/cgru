'''
# Example:

python3 -c 'import sys; sys.path.append("/data/cgru/rules"); import rulib; from rulib import playlist; pl=playlist.Playlist("/TEST/SHOTS/API");'

'''
import copy
import os
import time

import json
import rulib

def getPlaylistFilePath(i_location = None):
    return rulib.functions.getRuFilePath('playlist.json', i_location)

def getPlaylistFileObj(i_location = None, out = None):
    data = rulib.functions.readObj(getPlaylistFilePath(i_location), out)
    if data is None:
        return None
    if 'playlist' in data:
        return data
    error = 'Playlist data not found'
    if out is not None: out['error'] = error
    else: print(error)
    return  None

def savePlaylistObject(i_object, i_location, out=None):
    path = getPlaylistFilePath(i_location)
    rufolder = os.path.dirname(path)
    if not os.path.isdir(rufolder):
        try:
            os.makedirs(rufolder)
        except PermissionError:
            err = 'Permissions denied to make folder for status: ' + rufolder
            if out is not None: out['error'] = err
            else: print(err)
            return False
        except:
            err = 'Unable to create folder for status: ' + rufolder
            err += '\n%s' % traceback.format_exc()
            if out is not None: out['error'] = err
            else: print(err)
            return False
    return rulib.functions.writeObj(path, i_object, out)

class Playlist:

    def __init__(self, location=None, pinned=[]):

        self.location_objects = []
        locations = []

        location = rulib.functions.getRootPath(location)
        while True:
            obj = getPlaylistFileObj(location)
            if obj is not None:
                obj['location'] = location
                self.location_objects.append(obj)
                locations.append(location)
            if location == '/' or len(location) == 0:
                break
            location = os.path.dirname(location)

        for pin in sorted(pinned):
            location = rulib.functions.getRootPath(pin)
            while True:
                if location in locations:
                    break
                obj = getPlaylistFileObj(location)
                if obj is not None:
                    obj['location'] = location
                    #self.location_objects.append(obj)
                    self.location_objects.insert(0, obj)
                    locations.append(location)
                if location == '/' or len(location) == 0:
                    break
                location = os.path.dirname(location)


        if len(self.location_objects) == 0:
            obj = dict()
            obj['id'] = '/'
            obj['location'] = '/'
            obj['playlist'] = []
            self.location_objects = append(obj)


    def generatePlaylist(self):
        location_objects = copy.deepcopy(list(reversed(self.location_objects)))
        playlist = None
        upper_obj = None
        for obj in location_objects:
            if upper_obj is None:
                # This is a root playlist:
                upper_obj = obj
                playlist = obj
                continue
            find_obj = self.findLocationById(os.path.dirname(obj['location']), location_objects)
            if find_obj:
                upper_obj = find_obj
            upper_obj['playlist'].append(obj)
            upper_obj = obj
        return playlist

    
    def stdout(self, i_obj=None):
        if i_obj is None:
            i_obj = self.generatePlaylist()
        print(json.dumps(i_obj, indent=4))


    def printError(self, i_msg, o_out=None):
        if type(o_out) is dict:
            o_out['error'] = i_msg
        else:
            print('ERROR: ' + i_msg)


    def filterLabelToId(self, i_label):
        o_id = ''
        for c in i_label:
            if c.isalnum() or c in ['.-_']:
                o_id += c
            else:
                o_id += '_'
        return o_id


    def makeId(self, i_label, i_parent_id):
        return os.path.normpath(os.path.join(i_parent_id, self.filterLabelToId(i_label)))


    def findObjById(self, i_id, o_obj=None):
        if o_obj is None:
            for obj in self.location_objects:
                find_obj = self.findObjById(i_id, obj)
                if find_obj is not None:
                    return find_obj
        else:
            if o_obj['id'] == i_id:
                return o_obj
            if i_id.find(o_obj['id']) != 0:
                return None
            if 'playlist' in o_obj:
                for obj in o_obj['playlist']:
                    find_obj = self.findObjById(i_id, obj)
                    if find_obj is not None:
                        return find_obj

        return None


    def findLocationById(self, i_id, i_location_objects = None):
        location_objects = i_location_objects
        if location_objects is None:
            location_objects = self.location_objects
        id_path = i_id
        while True:
            for obj in location_objects:
                if obj['location'] == id_path:
                    return obj
            if id_path == '/' or len(id_path) == 0:
                break
            id_path = os.path.dirname(id_path)
        return None


    def createObj(self, label=None, id_parent=None, uid=None, out=None):
        if label is None:
            self.printError('Label is not specified', out)
            return None

        if id_parent is None:
            self.printError('Parent id is not specified', out)
            return None

        id = self.makeId(label, id_parent)

        if self.findObjById(id) is not None:
            self.printError('Object with id="%s" already exists.' % id, out)
            return None

        if uid is None:
            uid = rulib.functions.getCurUser()

        new_obj = dict()
        new_obj['id'] = id
        new_obj['label'] = label
        new_obj['user'] = uid
        new_obj['time'] = int(time.time())

        return new_obj


    def saveLocation(self, i_obj, o_out=None):
        return savePlaylistObject(i_obj, i_obj['location'], o_out)


    def saveLocationById(self, i_id, o_out=None):
        obj = self.findLocationById(i_id)
        if obj is None:
            self.printError('Can not find location for id = "%s"' % i_id, o_out)
            return False
        return self.saveLocation(obj, o_out)


    def addPaths(self, paths=[], id_parent=None, id_before=None, uid=None, out=None):
        if paths is None or len(paths) == 0:
            self.printError('Paths are not specified.', out)
            return

        parent_obj = None
        for path in paths:
            new_obj = self.createObj(label=path['label'], id_parent=id_parent, uid=uid, out=out)
            if new_obj is None:
                return
            new_obj['path'] = path['path']

            parent_obj = self.addObject(new_obj, id_parent=id_parent, id_before=id_before, out=out)
            if parent_obj is None:
                return

        self.saveLocationById(parent_obj['id'], out)


    def addFolder(self, label=None, id_parent=None, id_before=None, uid=None, out=None):
        new_obj = self.createObj(label=label, id_parent=id_parent, uid=uid, out=out)
        if new_obj is None:
            return
        new_obj['playlist'] = []

        parent_obj = self.addObject(new_obj, id_parent=id_parent, id_before=id_before, out=out)
        if parent_obj:
            self.saveLocationById(new_obj['id'], out)

        
    def addObject(self, i_obj, id_parent=None, id_before=None, out=None):

        parent_obj = self.findObjById(id_parent)
        if parent_obj is None:
            self.printError('Can not find parent object with id="%s".' % id_parent, out)
            return None

        if not 'playlist' in parent_obj:
            parent_obj['playlist'] = []

        if id_before is not None:
            if id_before == "":
                parent_obj['playlist'].append(i_obj)
                return parent_obj
            found = False
            for i, before_obj in enumerate(parent_obj['playlist']):
                if before_obj['id'] == id_before:
                    parent_obj['playlist'].insert(i, i_obj)
                    found = True
                    break
            if not found:
                self.printError('Object with id_before="%s" not found.' % id_before, out)
                return None
        else:
            # Insert aphabetically by label:
            found = False
            for i, before_obj in enumerate(parent_obj['playlist']):
                if before_obj['label'] >= i_obj['label']:
                    found = True
                    break
            if found:
                parent_obj['playlist'].insert(i, i_obj)
            else:
                parent_obj['playlist'].append(i_obj)

        return parent_obj


    def deleteObject(self, id=None, out=None, uid=None, i_obj=None):
        if i_obj is None:
            for i, lobj in enumerate(self.location_objects):
                if self.deleteObject(id, out, uid, lobj['playlist']):
                    self.saveLocation(lobj, out)
                    return

        else:
            found = False
            for i, obj in enumerate(i_obj):
                if obj['id'] == id:
                    found = True
                    del i_obj[i]
                    break
                elif 'playlist' in obj:
                    found = self.deleteObject(id, out, uid, obj['playlist'])
                    if found:
                        break
            return found

        self.printError('Object with id="%s" not found' % id, out)
 

    def renameObject(self, id=None, new_label=None, out=None, uid=None, i_obj=None):
        if i_obj is None:
            for i, lobj in enumerate(self.location_objects):
                if self.renameObject(id, new_label, out, uid, lobj['playlist']):
                    self.saveLocation(lobj, out)
                    return

        else:
            found = False
            for i, obj in enumerate(i_obj):
                if obj['id'] == id:
                    obj['label'] = new_label
                    found = True
                    break
                elif 'playlist' in obj:
                    found = self.renameObject(id, new_label, out, uid, obj['playlist'])
                    if found:
                        break
            return found

        self.printError('Object with id="%s" not found' % id, out)
 

    def moveObject(self, id=None, mode=None, out=None, uid=None, i_obj=None):
        if i_obj is None:
            for i, lobj in enumerate(self.location_objects):
                if self.moveObject(id, mode, out, uid, lobj['playlist']):
                    self.saveLocation(lobj, out)
                    return

        else:
            found = False
            for i, obj in enumerate(i_obj):
                if obj['id'] == id:
                    obj = i_obj.pop(i)
                    if mode == 'up' and i > 0:
                        i_obj.insert(i-1, obj)
                    elif mode == 'down' and i < len(i_obj):
                        i_obj.insert(i+1, obj)
                    elif mode == 'top':
                        i_obj.insert(0, obj)
                    elif mode == 'bottom':
                        i_obj.append(obj)
                    else:
                        self.printError('Invalid mode="%s", valid modes are: "up","down","top","bottom"' % mode, out)
                        i_obj.insert(i, obj)
                    found = True
                    break
                elif 'playlist' in obj:
                    found = self.moveObject(id, mode, out, uid, obj['playlist'])
                    if found:
                        break
            return found

        self.printError('Object with id="%s" not found' % id, out)


    def addLocation(self, location=None, out=None, uid=None):
        if location is None:
            self.printError('Location is not specified.', out)
            return
        if len(location) == 0:
            self.printError('Location is empty.', out)
            return
        if location[0] != '/':
            location = '/' + location
        if location == '/':
            self.printError('Can not create another root playlist="%s"' % location, out)
            return
        abspath = rulib.functions.getAbsPath(location)
        if not os.path.isdir(abspath):
            self.printError('Location does not exist: "%s".' % location, out)
            return
        if os.path.isfile(getPlaylistFilePath(location)):
            self.printError('Playlist at location="%s" already exists.' % location, out)
            return

        id_parent = os.path.dirname(location)
        label = os.path.basename(location)

        obj = self.createObj(label=label, id_parent=id_parent, uid=uid, out=out)
        if obj is None:
            return

        obj['location'] = location
        obj['playlist'] = []

        if self.saveLocation(obj, out):
            self.location_objects.insert(0, obj)

'''

"editobj":{"objects":[{"label":"SER_01_SHOT_0020","path":"/TEST/SHOTS/SER_01/SER_01_SHOT_0020","id":"/SER_01_SHOT_0020","user":"timurhai","time":1749921596}],"pusharray":"playlist","id_before":null,"id":"","file":"root/.rules/playlist.json"}

{"editobj":{"objects":[{"label":"qwer","id":"/qwer","user":"timurhai","time":1749997272,"playlist":[]}],"pusharray":"playlist","id":"","file":"root/.rules/playlist.json"}}

"editobj":{"objects":[{"id":"/SER_01_SHOT_0020"}],"delarray":"playlist","file":"root/.rules/playlist.json"}}

{"editobj":{"objects":[{"label":"qwer","id":"/qwer"}],"replace":true,"file":"root/.rules/playlist.json"}}

'''

