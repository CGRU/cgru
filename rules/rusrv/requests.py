import hashlib
import json
import os
import subprocess

from rusrv import environ
from rusrv import functions

def req_start(i_args, out):
    out['version'] = environ.CGRU_VERSION
    out['software'] = environ.SERVER_SOFTWARE
    out['client_ip'] = environ.REMOTE_ADDR
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
            user['avatar'] = 'https://gravatar.com/avatar/' + hashlib.md5(obj['email'].strip().tolower().encode()).hexdigest()

        out['users'][obj['id']] = user;

        # TODO
        #if (isAdmin($out)) $o_out['admin'] = true;


def req_getfile(i_file, out):
    if not os.path.isfile(i_file):
        out['error'] = 'No such file: ' + i_file
        return

    # TODO
    #if (false == htaccessPath($i_file))
    #   $o_out['error'] = 'Permissions denied';
    #   return;

    data = functions.fileRead(i_file)
    if data:
        return data

    out['error'] = 'Unable to load file: ' + i_file


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

