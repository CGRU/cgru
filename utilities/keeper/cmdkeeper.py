# -*- coding: utf-8 -*-

import json
import socket
import sys

import cgruconfig

from sendkeeper import sendkeeper

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

cmdexec = dict()
if Options.cmd:  cmdexec['cmds'] = [Options.cmd]
if Options.eval: cmdexec['eval'] = Options.eval
if Options.open: cmdexec['open'] = Options.open

if len(cmdexec) == 0:
    Parser.print_help()
    sys.exit(1)

sendkeeper(cmdexec)
