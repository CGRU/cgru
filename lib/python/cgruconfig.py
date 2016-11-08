#!/usr/bin/env python
# -*- coding: utf-8 -*-

import collections
import errno
import json
import os
import socket
import stat
import sys
import time

import cgruutils

VARS = dict()


def checkConfigFile(path):
    status = True
    if not os.path.isfile(path):
        try:
            cfile = open(path, 'w')
        except Exception as err:
            if err.errno == errno.EPERM or err.errno == errno.EACCES:
                print('Warning! Permission error while opening %s' % path)
            elif err.errno is errno.EROFS:
                print('Warning! Could not edit %s, read-only file system' % path)
            else:
                print('Warning! Unexpected error while opening %s.' % path)
                print('Error: %s' % err)
            status = False
        else:
            cfile.write('{"cgru_config":{\n')
            cfile.write(
                '"":"Created by CGRU Keeper at %s",\n' % time.ctime())
            cfile.write('"":""\n')
            cfile.write('}}\n')
            cfile.close()

    if status:
        try:
            os.chmod(path, stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)
        except Exception as err:
            if err.errno == errno.EPERM or err.errno == errno.EACCES:
                print('Warning! Could not modify permissions for %s, permission denied.' % path)
            else:
                print('Warning! Unexpected error while modifying the permissions for %s.' % path)
                print('Error: %s' % err)
    return status


class Config:
    def __init__(self, variables=VARS, configfiles=None, Verbose=False):
        self.verbose = Verbose
        self.Vars = variables
        self.recursion = False

        if self.verbose:
            print('Loading CGRU config...')

        if configfiles is None:
            self.recursion = True
            self.Vars['filenames'] = []

            self.Vars['platform'] = ['unix']
            if sys.platform[:3] == 'win':
                self.Vars['platform'] = ['windows']
            elif sys.platform[:6] == 'darwin':
                self.Vars['platform'].append('macosx')
            elif sys.platform[:5] == 'linux':
                self.Vars['platform'].append('linux')
            if self.verbose:
                print('Platform: "%s"' % ','.join( self.Vars['platform']))

            self.Vars['HOSTNAME'] = socket.gethostname().lower()

            try:
                cgrulocation = os.environ['CGRU_LOCATION']
                if cgrulocation is None or cgrulocation == '':
                    raise KeyError('CGRU_LOCATION')
            except KeyError as e:
                raise KeyError('Environment variable CGRU_LOCATION not set.')

            # Definitions which always must preset:
            self.Vars['CGRU_LOCATION'] = cgrulocation
            self.Vars['CGRU_VERSION'] = os.getenv('CGRU_VERSION', '')
            self.Vars['CGRU_PYTHONEXE'] = os.getenv('CGRU_PYTHONEXE', 'python')
            self.Vars['CGRU_UPDATE_CMD'] = os.getenv('CGRU_UPDATE_CMD')

            self.Vars['company'] = 'CGRU'
            self.Vars['menu_path'] = None
            self.Vars['tray_icon'] = None
            self.Vars['icons_path'] = None
            if sys.platform.find('win') == 0:
                self.Vars['editor'] = 'notepad "%s"'
            else:
                self.Vars['editor'] = 'xterm -e vi "%s"'

            afroot = os.getenv('AF_ROOT')
            if afroot is None:
                afroot = os.path.join(cgrulocation, 'afanasy')

            self.Vars['AF_ROOT'] = afroot

            username = os.getenv(
                'CGRU_USERNAME',
                os.getenv(
                    'AF_USERNAME',
                    os.getenv(
                        'USER',
                        os.getenv(
                            'USERNAME'
                        )
                    )
                )
            )

            if username is None:
                username = 'None'

            # cut DOMAIN from username:
            dpos = username.rfind('/')
            if dpos == -1:
                dpos = username.rfind('\\')

            if dpos != -1:
                username = username[dpos + 1:]

            username = username.lower()
            self.Vars['USERNAME'] = username

            home = os.getenv('APPDATA', os.getenv('HOME'))
            if home is None:
                home = username

            self.Vars['HOME'] = home

            if sys.platform.find('win') == 0:
                self.Vars['HOME_CGRU'] = os.path.join(home, 'cgru')
            else:
                self.Vars['HOME_CGRU'] = os.path.join(home, '.cgru')

            self.Vars['config_file_home'] = \
                os.path.join(self.Vars['HOME_CGRU'], 'config.json')

            if sys.platform.find('win') == 0 or os.geteuid() != 0:
                if not os.path.exists( self.Vars['HOME_CGRU']):
                    try:
                        os.makedirs(self.Vars['HOME_CGRU'])
                    except:
                        pass
                # Create cgru home config file if not preset
                checkConfigFile(self.Vars['config_file_home'])

            configfiles = []
            configfiles.append(
                os.path.join(cgrulocation, 'config_default.json')
            )
            configfiles.append(self.Vars['config_file_home'])

        for filename in configfiles:
            self.load(filename)

        # Get values overrides from environment:
        for name in self.Vars:

            env_name = 'CGRU_' + name.upper()
            env_val  = os.getenv( env_name)
            if env_val is None:
                continue

            if self.verbose:
                print('%s=%s' %  (env_name ,env_val))

            if isinstance( self.Vars[name], int):
                self.Vars[name] = int(env_val)
            elif isinstance( self.Vars[name], float):
                self.Vars[name] = float(env_val)
            elif isinstance( self.Vars[name], bool):
                self.Vars[name] = bool(env_val)
            else:
                self.Vars[name] = env_val

    def load(self, filename):
        if self.recursion:
            if filename in self.Vars['filenames']:
                print('ERROR: Config file already included:')
                print(filename)
                return
            self.Vars['filenames'].append(filename)

        if self.verbose:
            print('Trying to open %s' % filename)
        if not os.path.isfile(filename):
            return

        with open(filename, 'r') as f:
            filedata = f.read()

        success = True
        try:
            if sys.hexversion > 0x02070000:
                obj = json.loads( filedata, object_pairs_hook=collections.OrderedDict)['cgru_config']
            else:
                obj = json.loads( filedata)['cgru_config']
        except:  # TODO: Too broad exception clause
            success = False
            print(filename)
            print(str(sys.exc_info()[1]))

        if not success:
            return

        self.getVars(self.Vars, obj, filename)

        if 'include' in obj:
            for afile in obj['include']:
                afile = os.path.join(os.path.dirname(filename), afile)
                self.load(afile)
                continue


    def getVars(self, o_vars, i_obj, i_filename):
        for key in i_obj:
            if len(key) == 0:
                continue

            if key[0] == '-':
                continue

            if key[:3] == 'OS_':
                if key[3:] in VARS['platform']:
                    self.getVars(o_vars, i_obj[key], i_filename)
                continue

            if isinstance(i_obj[key], dict):
                if key in o_vars:
                    if isinstance(o_vars[key], dict):
                        self.getVars(o_vars[key], i_obj[key], i_filename)
                        continue

            if self.verbose:
                print('    ' + key + ': ' + str(i_obj[key]))
            o_vars[key] = i_obj[key]


if len(VARS) == 0:
    Config()

def reconfigure():
    VARS = dict()
    Config()

def writeVars(variables, configfile=VARS['config_file_home']):
    with open(configfile, 'r') as file_:
        lines = file_.readlines()
        file_.close()
        for var in variables:
            tofind = '"%s":' % var
            toinsert = '\t"%s":%s,\n' % (var, json.dumps(VARS[var]))
            found = False
            num = -1
            for line in lines:
                num += 1
                if line.find(tofind) == -1:
                    continue
                found = True
                lines[num] = toinsert
                break
            if not found:
                num = 1
                for line in lines:
                    num += 1
                    if line.find('"":""') != -1:
                        continue
                    lines.insert(num, toinsert)
                    break

        file_ = open(configfile, 'w')

        for line in lines:
            file_.write(line)
