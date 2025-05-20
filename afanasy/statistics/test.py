#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os
import sys
import time

sys.path.append(os.getcwd())

import afstatsrv

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

out = request('init')
folder = out['folders'][0]
print('# Folder = "%s"' % folder)

args = dict()
args['select'] = 'folder'
#args['time_max'] = int(time.time())
args['time_min'] = int(time.time()) - 60*60 * 24 * 48
args['folder'] = folder
out = request('get_jobs_folders', args)

args['favorite'] = 'username'
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
