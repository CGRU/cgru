#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os
import sys
import time

sys.path.append(os.getcwd())

import afstatsrv

r = afstatsrv.Requests()

out = dict()
r.req_init(None, out)
print(json.dumps(out, indent=4))
folder = out['folders'][0]
print('Folder = "%s"' % folder)

args = dict()
args['table'] = 'jobs'
args['select'] = 'folder'
args['time_max'] = int(time.time())
args['time_min'] = args['time_max'] - 60*60 * 24 * 48
args['folder'] = folder
out = dict()
r.req_get_jobs_folders(args, out)
print(json.dumps(out, indent=4))

args['favorite'] = 'username'
out = dict()
r.req_get_jobs_table(args, out)
print(json.dumps(out, indent=4))

out = dict()
r.req_get_tasks_folders(args, out)
print(json.dumps(out, indent=4))

out = dict()
r.req_get_tasks_table(args, out)
print(json.dumps(out, indent=4))

args['interval'] = 1000000
out = dict()
r.req_get_tasks_folders_graph(args, out)
print(json.dumps(out, indent=4))

out = dict()
r.req_get_tasks_graph(args, out)
print(json.dumps(out, indent=4))

out = dict()
args['folder'] = '/xxx'
r.req_folder_delete(args, out)
print(json.dumps(out, indent=4))
