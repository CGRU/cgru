#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import socket
import stat
import sys
import time
import json

import cgruutils

VARS = dict()


def checkConfigFile(path):
    status = True
    if not os.path.isfile(path):
        try:
            cfile = open(path, 'w')
        except:  # TODO: Too broad exception clause
            print(str(sys.exc_info()[1]))
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
        except:  # TODO: Too broad exception clause
            print(str(sys.exc_info()[1]))
    return status


class Config:
    def __init__(self, variables=VARS, configfiles=None, Verbose=False):
        self.verbose = Verbose
        self.Vars = variables
        self.recursion = False

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
                print('Platform: "%s"' % self.Vars['platform'].join(' '))

            self.Vars['HOSTNAME'] = socket.gethostname().lower()

            cgrulocation = os.getenv('CGRU_LOCATION')
            if cgrulocation is None or cgrulocation == '':
                return

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
                cgruutils.createFolder(self.Vars['HOME_CGRU'])
                # Create cgru home config file if not preset
                checkConfigFile(self.Vars['config_file_home'])

            configfiles = []
            configfiles.append(
                os.path.join(cgrulocation, 'config_default.json')
            )
            configfiles.append(self.Vars['config_file_home'])

        for filename in configfiles:
            self.load(filename)

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

        file = open(filename, 'r')
        filedata = file.read()
        file.close()

        success = True
        try:
            obj = json.loads(filedata)['cgru_config']
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

            if type(i_obj[key]) == type(dict()):
                if key in o_vars:
                    if type(o_vars[key]) == type(dict()):
                        self.getVars(o_vars[key], i_obj[key], i_filename)
                        continue

            o_vars[key] = i_obj[key]


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
