#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os
import sys
import time
from datetime import datetime

sys.path.append(os.getcwd())

import afstatsrv

from optparse import OptionParser
Parser = OptionParser(usage="%prog [Options]\nType \"%prog -h\" for help", version="%prog 1.0")
Parser.add_option('-t', '--table',    dest='table',    type   = 'string',     default = None,       help = 'Table: logs, jobs, tasks')
Parser.add_option(      '--tmin',     dest='timemin',  type   = 'string',     default = None,       help = 'Record minimum time: 2026-02-23')
Parser.add_option(      '--tmax',     dest='timemax',  type   = 'string',     default = None,       help = 'Record maximum time: 2026-03-08')
Parser.add_option(      '--select',   dest='select',   type   = 'string',     default = 'folder',   help = 'Select column')
Parser.add_option('-l', '--like',     dest='like',     type   = 'string',     default = None,       help = 'Select like: -l subject:afanasy')
Parser.add_option(      '--favorite', dest='favorite', type   = 'string',     default = 'username', help = 'Favourite column')
Parser.add_option(      '--delete',   dest='delete',   action = 'store_true', default = False,      help = 'Delete records')
Parser.add_option('-V', '--verbose',  dest='verbose',  action = 'store_true', default = False,      help = 'Verbose mode')
Parser.add_option('-D', '--debug',    dest='debug',    action = 'store_true', default = False,      help = 'Debug mode')
Options, Args = Parser.parse_args()

def errExit(i_msg):
    print('ERROR: ' + i_msg)
    sys.exit(1)

TimeMin = Options.timemin
TimeMax = Options.timemax
if TimeMin:
    TimeMin = int(datetime.fromisoformat(TimeMin).timestamp())
if TimeMax:
    TimeMax = int(datetime.fromisoformat(TimeMax).timestamp())

r = afstatsrv.Requests()

def request(i_name, i_args = None):
    func = 'req_' + i_name
    print('\n' + '#'*8 + ' ' + func)
    print(json.dumps(i_args, indent=4))
    out = dict()
    getattr(r, func)(i_args, out)
    print('# out:')
    print(json.dumps(out, indent=4))
    return out

def showDemo():
    out = request('init')
    folder = out['folders'][0]
    print('# Folder = "%s"' % folder)

    args = dict()
    if TimeMin:
        args['time_min'] = TimeMin
    if TimeMax:
        args['time_max'] = TimeMax
    args['select'] = Options.select
    args['folder'] = folder
    out = request('get_jobs_folders', args)

    args['favorite'] = Options.favorite
    out = request('get_jobs_table', args)

    out = request('get_tasks_folders', args)

    out = request('get_tasks_table', args)

    args['interval'] = 1000000
    out = request('get_tasks_folders_graph', args)

    out = request('get_tasks_graph', args)

    args['folder'] = '/xxx'
    out = request('folder_delete', args)

    args['order'] = 'time'
    out = request('get_logs_table', args)

if Options.table:
    args = dict()
    args['table']    = Options.table
    args['select']   = Options.select
    args['favorite'] = Options.favorite
    args['folder']   = '/'

    if TimeMin:
        args['time_min'] = TimeMin
    if TimeMax:
        args['time_max'] = TimeMax

    if Options.like:
        col = Options.like.split('=')[0]
        val = Options.like.split('=')[1]
        like = dict()
        like[col] = val
        args['like'] = like

    reqest_name = 'get_%s_table' % Options.table

    if Options.delete:
        if TimeMax is None:
            errExit('TIme Max should be specified to delete records ("--tmax").')
        reqest_name = 'table_delete'

    out = request(reqest_name, args)
else:
    showDemo()
