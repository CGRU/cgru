# -*- coding: utf-8 -*-

import json
import socket
import sys

import cgruconfig

from optparse import OptionParser
Parser = OptionParser( usage="%prog [options]\ntype \"%prog -h\" for help", version="%prog 1.  0")
Parser.add_option('-c', '--cmd',  dest='cmd',  type='string', default=None, help='System command to launch.')
Parser.add_option('-e', '--eval', dest='eval', type='string', default=None, help='Python function to eval, see "cmd.py" file.')
Parser.add_option('-o', '--open', dest='open', type='string', default=None, help='Open folder.')
Parser.add_option('-P', '--port', dest='port', type='string', default=None, help='Keeper port (default from config).')
Parser.add_option('-H', '--host', dest='host', type='string', default=None, help='Keeper host (default "localhost").')
(Options, Args) = Parser.parse_args()

if Options.port is None: Options.port = cgruconfig.VARS['keeper_port']
if Options.host is None: Options.host = 'localhost'

obj = dict()

obj['headers'] = dict()
obj['headers']['username'] = cgruconfig.VARS['USERNAME']
obj['headers']['hostname'] = cgruconfig.VARS['HOSTNAME']

obj['cmdexec'] = dict()
if Options.cmd:  obj['cmdexec']['cmds'] = [Options.cmd]
if Options.eval: obj['cmdexec']['eval'] = Options.eval
if Options.open: obj['cmdexec']['open'] = Options.open

if len(obj['cmdexec']) == 0:
    Parser.print_help()
    sys.exit(1)

data = json.dumps( obj, sort_keys=True, indent=4)
print(data)

s = None
for res in socket.getaddrinfo( Options.host, Options.port, socket.AF_UNSPEC, socket.SOCK_STREAM):
    family, socktype, proto, canonname, sockaddr = res
    print('Trying to connect to "%s"' % str(sockaddr[0]))

    try:
        s = socket.socket(family, socktype, proto)
    except Exception as e:
        print(str(e))
        s = None
        continue

    try:
        s.connect(sockaddr)
    except Exception as e:
        print(str(e))
        s.close()
        s = None
        continue
    break

if s is None:
    print('Could not open socket.')
else:
    s.sendall( bytearray(data, 'utf-8'))

