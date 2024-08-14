import os
import shutil
import time
import traceback

import rulib

from rusrv import environ
from rulib import functions

class Admin:

    def __init__(self, i_session):
        self.session = i_session


    def processUser(self, i_arg, o_out):
        if not os.path.isdir('users'):
            os.mkdir('users')

        if self.session.USER_ID is None:
            return

        user = functions.readUser(self.session.USER_ID, True)
        if user is None:
            user = dict()

        if 'error' in user:
            o_out['error'] = user['error']

        user['rtime'] = int(time.time())
        if not 'id'       in user: user['id']       = self.session.USER_ID
        if not 'channels' in user: user['channels'] = []
        if not 'news'     in user: user['news']     = []
        if not 'ctime'    in user: user['ctime']    = user['rtime']

        self.processUserIP(i_arg, user)

        # Delete nulls from some arrays:
        arrays = ['news', 'bookmarks', 'channels']
        for arr in arrays:
            if arr in user and user[arr] is not None:
                user[arr] = list(filter(lambda a: a != None, user[arr]))

        if not functions.writeUser(user, False):
            o_out['error'] = 'Can`t write current user'
            return

        o_out['user'] = user;


    def processUserIP(self, i_args, o_user):
        ip = self.session.REMOTE_ADDR

        if not 'ips' in o_user:
            o_user['ips'] = []

        # Remove other enties from this IP:
        o_user['ips'] = list(filter(lambda a: a['ip'] != ip, o_user['ips']))

        entry = dict()
        entry['ip'] = ip
        entry['time'] = int(time.time())
        if 'url' in i_args:
            entry['url'] = i_args['url']

        # Insest and entry in the beginning
        o_user['ips'].insert(0, entry)

        # Limit entries count 
        o_user['ips'] = o_user['ips'][:10]


    def isAdmin(self, o_out = None):

        if self.session.USER_ID is None:
            if o_out is not None:
                o_out['error'] = 'Access denied.'
            return False

        if self.session.GROUPS is None:
            out = dict()
            self.readGroups(out)
            if 'error' in out:
                if o_out is not None:
                    o_out['error'] = out['error']
                return False

            if self.session.GROUPS is None:
                if o_out is not None:
                    o_out['error'] = 'Error reading groups.'
                return False

        if not 'admins' in self.session.GROUPS:
            if o_out is not None:
                o_out['error'] = 'No admins group found.'
            return False

        if self.session.USER_ID in self.session.GROUPS['admins']:
            return True

        if o_out is not None:
            o_out['error'] = 'Access denied.'

        return False

    def readGroups(self, o_out = None):

        if self.session.GROUPS is not None:
            return True

        if not os.path.isfile(rulib.HT_GROUPS_FILE_NAME):
            error = 'HT Groups file does not exist.'
            if o_out:
                o_out['error'] = error
            else:
                print(error)
            return False

        data = functions.fileRead(rulib.HT_GROUPS_FILE_NAME)
        if data is None:
            error = 'Unable to read groups file.'
            if o_out:
                o_out['error'] = error
            else:
                print(error)
            return False

        groups = dict()

        for line in data.split('\n'):
            if len(line) < 3:
                continue

            fields = line.split(':')
            if len(fields) == 0:
                continue
            if len(fields[0]) < 1:
                continue

            groups[fields[0]] = []
            if len(fields) < 2:
                continue

            for user in fields[1].split(' '):
                if len(user) < 1:
                    continue

                groups[fields[0]].append(user)

        self.session.GROUPS = groups

        return True


    def permissionsGet(self, i_args, o_out):

        o_out['groups'] = []
        o_out['users'] = []
        o_out['merge'] = False

        if not os.path.isdir(i_args['path']):
            o_out['error'] = 'No such directory.'
            return

        htaccess = os.path.join(i_args['path'], rulib.HT_ACCESS_FILE_NAME)
        if not os.path.isfile(htaccess):
            return

        data = functions.fileRead(htaccess)
        if data is None:
            o_out['error'] = 'Can`t read the file.'
            return

        for line in data.split('\n'):

            if len(line) <= 1:
                continue

            words = line.split(' ')

            if len(words) < 2:
                o_out['error'] = 'Invalid line: "%s"' % line
                return

            if words[0] == 'AuthMerging':
                if words[1] == 'Or':
                    o_out['merge'] = True
                continue

            if words[0] != 'Require':
                continue

            del words[0]

            if words[0] == 'group':
                del words[0]
                for group in words:
                    o_out['groups'].append(group)
            elif words[0] == 'user':
                del words[0]
                for user in words:
                    o_out['users'].append(user)
            elif words[0] == 'valid-user':
                o_out['valid_user'] = True


    def permissionsSet(self, i_args, o_out):

        # There should be at least one group 'admins'
        if not 'groups' in i_args:
            i_args['groups'] = []
        if not 'admins' in i_args['groups']:
            i_args['groups'].insert(0, 'admins')

        lines = []

        path = os.path.normpath(i_args['path'])
        if len(path.split('/')) > 1:
            lines.append('AuthMerging Or')

        if 'groups' in i_args and len(i_args['groups']):
            lines.append('Require group ' + ' '.join(i_args['groups']))

        if 'users' in i_args and len(i_args['users']):
            lines.append('Require user ' + ' '.join(i_args['users']))

        data = '\n'.join(lines) + '\n'

        htaccess = os.path.join(path, rulib.HT_ACCESS_FILE_NAME)
        if not functions.fileWrite(htaccess, data):
            o_out['error'] = 'Unable to write into the file.'


    def permissionsClear(self, i_args, o_out):
        htaccess = os.path.join(i_args['path'], rulib.HT_ACCESS_FILE_NAME)
        if not os.path.isfile(htaccess):
            o_out['error'] = 'No permissions settings found.'
            return

        try:
            os.remove(htaccess)
        except:
            o_out['error'] = 'Can`t remove the file.'
            o_out['info'] = '%s' % traceback.format_exc()


    def htaccessFolder(self, i_folder):

        if i_folder == '.':
            return True

        if not self.readGroups():
            return False

        args = dict()
        args['path'] = i_folder
        out = dict()
        self.permissionsGet(args, out)

        if 'error' in out:
            print(out['error'])
            return False
        if 'valid_user' in out:
            if self.session.USER_ID is None:
                return False
            return True

        if len(out['users']) == 0 and len(out['groups']) == 0:
            return None

        if self.session.USER_ID is None:
            return False

        for grp in out['groups']:
            if grp in self.session.GROUPS:
                if self.session.USER_ID in self.session.GROUPS[grp]:
                    return True

        if self.session.USER_ID in out['users']:
            return True

        if out['merge']:
            return None

        return False


    def htaccessPath(self, i_path):

        if os.path.isfile(i_path):
            i_path = os.path.dirname(i_path)

        if not os.path.isdir(i_path):
            return False

        path = None
        paths = []
        for folder in i_path.split('/'):
            if path is not None:
                path = os.path.join(path, folder)
            else:
                path = folder
            paths.append(path)

        for path in reversed(paths):
            access = self.htaccessFolder(path)
            if access is False: return False
            if access is True: return True

        return True


    def disableUser(self, i_args, o_out):

        uid = i_args['uid']
        udir = os.path.join('users', uid)
        ufile = os.path.join(udir, uid+'.json')
        if not os.path.isfile(ufile):
            o_out['error'] = 'User file does not exist.'
            return

        # If user new object provided, we write it.
        # This needed to just disable user and not to loose its settings.
        if 'uobj' in i_args:
            if functions.writeUser(i_args['uobj'], True):
                o_out['status'] = 'success'
            else:
                o_out['error'] = 'Unable to write "%s" user file' % uid
        else:
            # Delete user files and loose all its data:
            shutil.rmtree(udir)

        # Remove user from digest file
        data = functions.fileRead(rulib.HT_DIGEST_FILE_NAME, True)
        if data is None:
            o_out['error'] = 'Unable to read the file.'
            return

        old_lines = data.split('\n')
        new_lines = []
        for line in old_lines:
            values = line.split(':')
            if len(values) == 3:
                if values[0] != uid:
                    new_lines.append(line)

        data = '\n'.join(new_lines) + '\n'

        if not functions.fileWrite(rulib.HT_DIGEST_FILE_NAME, data):
            o_out['error'] = 'Unable to write into the file.'


    def writeGroups(self, i_groups, o_out):
        self.session.GROUPS = i_groups

        lines = []
        for group in i_groups:
            lines.append(group + ':' + ' '.join(i_groups[group]))

        data = '\n'.join(lines) + '\n'
        if not functions.fileWrite(rulib.HT_GROUPS_FILE_NAME, data):
            o_out['error'] = 'Unable to write in groups file.'


    def htdigest(self, i_args, o_out):
        user = i_args['user']

        # Not admin can change only own password,
        # and only if he has a special state "passwd".
        # Admin can change any user password.
        if not self.isAdmin(o_out):
            if user != self.session.USER_ID:
                o_out['error'] = 'User can`t change other user password'
                return

            # Check "passwd" state:
            out = dict()
            functions.readAllUsers(out, False)
            if 'error' in out:
                o_out['error'] = out['error']
                return

            if not user in out['users']:
                o_out['error'] = 'User "%s" not found.' % user
                return

            uobj = out['users'][user]
            if (not 'states' in uobj) or (not 'passwd' in uobj['states']):
                o_out['error'] = 'You are not allowed to change password.'
                return

        data = functions.fileRead(rulib.HT_DIGEST_FILE_NAME, True)
        if data is None:
            data = ''

        # Construct new lines w/o our user (if it exists):
        o_out['status'] = 'User "%s" set.' % user
        o_out['user'] = user
        new_lines = []
        for line in data.split('\n'):
            values = line.split(':')
            if len(values) == 3:
                if values[0] == user:
                    # Just skip old line with our user:
                    o_out['status'] = 'User "%s" updated.' % user
                else:
                    # Store line with other user:
                    new_lines.append(line)

        # Add our user at the end:
        new_lines.append(i_args['digest'])

        data = '\n'.join(new_lines) + '\n'
        if not functions.fileWrite(rulib.HT_DIGEST_FILE_NAME, data):
            o_out['error'] = 'Unable to write into the file.'


    def http_digest_validate(self, i_digest, o_out):

        if not os.path.isfile(rulib.HT_DIGEST_FILE_NAME):
            o_out['error'] = 'HT digest file does not exist.'
            return False

        data = functions.fileRead(rulib.HT_DIGEST_FILE_NAME, True)
        if data is None:
            o_out['error'] = 'Can`t read HT digest file.'
            error_log(o_out['error'])
            return False

        data = data.split('\n')
        found = False
        for line in data:
            if line.find(i_digest['username']) == 0:
                data = line
                found = True
                break

        if not found:
            o_out['error'] = 'Wrong!'
            return False

        data = data.split(':')
        if len(data) != 3:
            o_out['error'] = 'Invalid HT digest entry.'
            return False

        print(i_digest)

        for field in ['uri','nonce','nc','cnonce','qop']:
            if not field in  i_digest:
                o_out['error'] = 'Received diges does not contain "%s" field.' % field
                return False

        data = data[2]
        hashlib = __import__('hashlib', globals(), locals(), [])
        hget = hashlib.md5(('POST:' + i_digest['uri']).encode()).hexdigest()
        data = data + ':' + i_digest['nonce'] + ':' + str(i_digest['nc']) + ':' + i_digest['cnonce'] + ':' + i_digest['qop'] + ':' + hget
        valid_response = hashlib.md5(data.encode()).hexdigest()

        if i_digest['response'] == valid_response:
            return True

        o_out['error'] = 'Wrong!'
        return False
