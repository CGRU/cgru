import base64
import hashlib
import json
import os
import subprocess
import sys
import traceback

from rusrv import environ
from rusrv import editobj
from rusrv import functions
from rusrv import news
from rusrv import search

class Requests:

    def __init__(self, i_session, i_admin):
        self.session = i_session
        self.admin = i_admin


    def req_start(self, i_args, out):
        out['version'] = environ.CGRU_VERSION
        out['software'] = environ.SERVER_SOFTWARE
        out['mod_wsgi_version'] = environ.MOD_WSGI_VERSION
        out['python'] = sys.version
        out['client_ip'] = self.session.REMOTE_ADDR
        out['auth_type'] = environ.AUTH_TYPE
        out['counter'] = environ.COUNTER
        if environ.AUTH_TYPE:
            out['AUTH_TYPE'] = environ.AUTH_TYPE
        if environ.AUTH_RULES:
            out['AUTH_RULES'] = environ.AUTH_RULES
            out['nonce'] = functions.randMD5()


    def req_initialize(self, i_args, out):
        configs = dict()
        functions.readConfig('config_default.json', configs)
        out['config'] = configs

        self.admin.processUser(i_args, out)
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

            if self.admin.isAdmin():
                out['admin'] = True


    def req_getfile(self, i_file, out):
        if not os.path.isfile(i_file):
            out['error'] = 'No such file: ' + i_file
            return

        if not self.admin.htaccessPath(i_file):
            o_out['error'] = 'Permissions denied';
            return

        data = functions.fileRead(i_file)
        if data:
            return data

        out['error'] = 'Unable to load file: ' + i_file


    def req_save(self, i_save, o_out):
        filename = i_save['file']
        dirname = os.path.dirname(filename)

        if self.session.USER_ID is None:
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
                data = base64.b64decode(data)

        if not functions.fileWrite(filename, data):
            o_out['error'] = 'Unable to open save file: ' + filename


    def req_permissionsget(self, i_args, o_out):
        if not self.admin.isAdmin(o_out):
            return
        return self.admin.permissionsGet(i_args, o_out)

    def req_permissionsset(self, i_args, o_out):
        if not self.admin.isAdmin(o_out):
            return
        return self.admin.permissionsSet(i_args, o_out)

    def req_permissionsclear(self, i_args, o_out):
        if not self.admin.isAdmin(o_out):
            return
        return self.admin.permissionsClear(i_args, o_out)

    def req_getallgroups(self, i_args, o_out):
        if not self.admin.isAdmin(o_out):
            return
        o_out['groups'] = self.session.GROUPS

    def req_getallusers(self, i_args, o_out):
        if not self.admin.isAdmin(o_out):
            return
        o_out['users'] = functions.readAllUsers(o_out, True)

    def req_disableuser(self, i_args, o_out):
        if not self.admin.isAdmin(o_out):
            return
        return self.admin.disableUser(i_args, o_out)

    def req_writegroups(self, i_args, o_out):
        if not self.admin.isAdmin(o_out):
            return
        return self.admin.writeGroups(i_args, o_out)

    def req_htdigest(self, i_args, o_out):
        if self.session.USER_ID is None:
            o_out['error'] = 'Guests can`t change any password'
            return
        return self.admin.htdigest(i_args, o_out)


    def req_cmdexec(self, i_args, o_out):
        if self.session.USER_ID is None:
            o_out['error'] = 'Guests are not allowed to run commands.'
            return

        o_out['cmdexec'] = []
        for cmd in i_args['cmds']:
            for rem in ['../', '../', '..', '&', '|', '>', '<']:
                cmd = cmd.replace(rem, '')

            out = None
            try:
                out = subprocess.check_output('./' + cmd, shell=True, stderr=subprocess.STDOUT, encoding='utf-8')
            except subprocess.CalledProcessError as e:
                if not 'ignore_errors' in i_args or i_args['ignore_errors'] is False:
                    obj = dict()
                    obj['error'] = e.output
                    if out is not None and len(out):
                        obj['error'] = out
                    obj['info'] = '%s' % traceback.format_exc()
                    o_out['cmdexec'].append(obj)
                    out = None

            if out is not None:
                obj = None
                try:
                    obj = json.loads(out)
                except:
                    obj = None
                if obj is not None:
                    out = obj

            o_out['cmdexec'].append(out)


    def req_editobj(self, i_edit, o_out):

        if self.session.USER_ID is None:
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
                    editobj.replaceObject(obj, newobj, i_edit)
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


    def req_makefolder(self, i_args, o_out):
        dirname = i_args['path']

        try:
            os.makedirs(os.fsdecode(dirname.encode()), 0o777)
        except:
            o_out['error'] = 'Unable to create directory ' + dirname
            o_out['info'] = '%s' % traceback.format_exc()
            return

        o_out['makefolder'] = dirname


    def req_makenews(self, i_args, o_out):
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
                ids = news.makebookmarks(self.session.USER_ID, bm, users, o_out)
                if 'error' in o_out:
                    return

                for id in ids:
                    if not id in users_changed:
                        users_changed.append(id)

        # Write changed users:
        for id in users_changed:
            functions.writeUser(users[id], True)

        o_out['users'] = users_changed

    def req_search(self, i_args, o_out):
        if not 'path' in i_args:
            o_out['error'] = 'Search path is not specified.'
            return

        path = i_args['path']
        for rem in ['../', '../', '..']:
            path = path.replace(rem, '')
        if not os.path.isdir(path):
            o_out['error'] = 'Search path does not exist: ' + path
            return

        if not self.admin.htaccessPath(path):
            o_out['error'] = 'Access denied.'
            return

        if not 'depth' in i_args:
            i_args['depth'] = 1

        o_out['search'] = i_args
        o_out['result'] = []

        search.search(i_args, o_out, path, 0)
