import base64
import hashlib
import json
import os
import subprocess
import sys
import traceback

from rusrv import admin
from rusrv import environ
from rusrv import editobj
from rusrv import functions
from rusrv import news

def req_start(i_args, out):
    out['version'] = environ.CGRU_VERSION
    out['software'] = environ.SERVER_SOFTWARE
    out['mod_wsgi_version'] = environ.MOD_WSGI_VERSION
    out['python'] = sys.version
    out['client_ip'] = environ.REMOTE_ADDR
    out['auth_type'] = environ.AUTH_TYPE
    out['counter'] = environ.COUNTER

def req_initialize(i_args, out):
    configs = dict()
    functions.readConfig('config_default.json', configs)
    out['config'] = configs

    functions.processUser(i_args, out)
    if 'error' in out:
        return

    users = functions.readAllUsers(out, False)
    if 'error' in out:
        return

    out['users'] = dict()
    for key in users:
        user = dict()
        obj = users[key]
        user['id'] = obj['id']

        # We do not send all users props to each user.
        props = ['title', 'role', 'tag', 'states', 'disabled', 'signature']
        for prop in props:
            if prop in obj:
                user[prop] = obj[prop]

        if 'avatar' in obj and len(obj['avatar']):
            user['avatar'] = obj['avatar']
        elif 'email' in obj and len(obj['email']):
            user['avatar'] = 'https://gravatar.com/avatar/' + hashlib.md5(obj['email'].strip().lower().encode()).hexdigest()

        out['users'][obj['id']] = user;

        if admin.isAdmin():
            out['admin'] = True


def req_getfile(i_file, out):
    if not os.path.isfile(i_file):
        out['error'] = 'No such file: ' + i_file
        return

    if not admin.htaccessPath(i_file):
        o_out['error'] = 'Permissions denied';
        return

    data = functions.fileRead(i_file)
    if data:
        return data

    out['error'] = 'Unable to load file: ' + i_file


def req_save(i_save, o_out):
    filename = i_save['file']
    dirname = os.path.dirname(filename)

    if environ.USER_ID is None:
        if os.path.isfile(filename) or ps.path.basename(filename) != 'body.html':
            o_out['error'] = 'Guests are not allowed to save files.'
            return

    o_out['save'] = filename

    if not os.path.isdir(dirname):
        try:
            os.makedirs(dirname, mode=0o777)
        except:
            o_out['error'] = '%s' % traceback.format_exc()
            return

    data = i_save['data']
    if 'type' in i_save:
        if i_save['type'] == 'base64':
            data = base65.b64decode(data)

    if not functions.fileWrite(filename, data):
        o_out['error'] = 'Unable to open save file: ' + filename


def req_permissionsget(i_args, o_out):
    if not admin.isAdmin(o_out):
        return
    return admin.permissionsGet(i_args, o_out)

def req_permissionsset(i_args, o_out):
    if not admin.isAdmin(o_out):
        return
    return admin.permissionsSet(i_args, o_out)

def req_permissionsclear(i_args, o_out):
    if not admin.isAdmin(o_out):
        return
    return admin.permissionsClear(i_args, o_out)

def req_getallgroups(i_args, o_out):
    if not admin.isAdmin(o_out):
        return
    o_out['groups'] = environ.GROUPS

def req_getallusers(i_args, o_out):
    if not admin.isAdmin(o_out):
        return
    o_out['users'] = functions.readAllUsers(o_out, True)

def req_disableuser(i_args, o_out):
    if not admin.isAdmin(o_out):
        return
    return admin.disableUser(i_args, o_out)

def req_writegroups(i_args, o_out):
    if not admin.isAdmin(o_out):
        return
    return admin.writeGroups(i_args, o_out)

def req_htdigest(i_args, o_out):
    if environ.USER_ID is None:
        o_out['error'] = 'Guests can`t change any password'
        return
    return admin.htdigest(i_args, o_out)


def req_cmdexec(i_args, o_out):
    if environ.USER_ID is None:
        o_out['error'] = 'Guests are not allowed to run commands.'
        return

    o_out['cmdexec'] = []
    for cmd in i_args['cmds']:
        for rem in ['../', '../', '..', '&', '|', '>', '<']:
            cmd = cmd.replace(rem, '')

        out = subprocess.check_output('./' + cmd, shell=True, encoding='utf-8')
        obj = None

        try:
            obj = json.loads(out)
        except:
            obj = None

        if obj is not None:
            out = obj

        o_out['cmdexec'].append(out)


def req_editobj(i_edit, o_out):

    if environ.USER_ID is None:
        if os.path.isfile(i_edit['file']):
            if not os.path.basename(i_edit['file']) in environ.GUESTCANEDIT:
                o_out['error'] = 'Guests are not allowed to edit here.'
                return
        else:
            if not os.path.basename(i_edit['file']) in environ.GUESTCANCREATE:
                o_out['error'] = 'Guests are not allowed here.'
                return

    # Read object:
    obj = functions.readObj(i_edit['file'])

    # Edit object:
    if 'add' in i_edit and i_edit['add'] is True:
        if obj is None:
            # This is a new object creation
            obj = dict()
            # Create folder if does not exist
            if not os.path.isdir(os.path.dirname(i_edit['file'])):
                os.makedirs(os.path.dirname(i_edit['file']), mode=0o777);
        editobj.mergeObjs(obj, i_edit['object'])
    else:
        if obj is None:
            # Object to edit does not exist
            o_out['status'] = 'error';
            o_out['error'] = 'Can`t edit null object: ' + i_edit['file']
            return

        if 'pusharray' in i_edit:
            editobj.pushArray(obj, i_edit)
        elif 'replace' in i_edit and i_edit['replace'] is True:
            for newobj in i_edit['objects']:
                editobj.replaceObject(obj, newobj)
        elif 'delarray' in i_edit:
            editobj.delArray(obj, i_edit)
        else:
            o_out['status'] = 'error'
            o_out['error'] = 'Unknown edit object operation: ' + i_edit['file']
            return

    # Write object:
    if functions.writeObj(i_edit['file'], obj):
        o_out['status'] = 'success'
        o_out['object'] = obj
    else:
        o_out['status'] = 'error'
        o_out['error'] = 'Can`t write to ' + i_edit['file']


def req_makenews(i_args, o_out):
    # Read all users:
    users = functions.readAllUsers(o_out, True)
    if 'error' in o_out:
        return

    if len(users) == 0:
        o_out['error'] = 'No users found.'
        return

    users_changed = []

    for request in i_args['news_requests']:
        ids = news.makenews(request, users, o_out)
        if 'error' in o_out:
            return

        for id in ids:
            if not id in users_changed:
                users_changed.append(id)

    if 'bookmarks' in i_args:
        for bm in i_args['bookmarks']:
            ids = news.makebookmarks(bm, users, o_out)
            if 'error' in o_out:
                return

            for id in ids:
                if not id in users_changed:
                    users_changed.append(id)

    # Write changed users:
    for id in users_changed:
        functions.writeUser(users[id], True)

    o_out['users'] = users_changed

