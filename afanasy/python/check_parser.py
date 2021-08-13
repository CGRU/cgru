#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os
import subprocess
import sys

import cgruutils

def UsageExit(message=''):
    """Missing DocString

    :param message:
    :return:
    """
    if message != '':
        print('Error: %s' % message)

    print('Usage: %s [parser type] [frames number] [command line]' %
          os.path.basename(sys.argv[0]))
    exit(1)


if len(sys.argv) < 4:
    UsageExit()

try:
    framesNum = int(sys.argv[2])
except:  # TODO: Too broad exception clause
    UsageExit('%s\nInvalid frames number.' % str(sys.exc_info()[1]))

parserType = sys.argv[1]
parserModule = __import__('parsers', globals(), locals(), [parserType])
cmd = 'parserModule.%s.%s()' % (parserType, parserType)
print(cmd)
parser = eval(cmd)

taskInfo = dict()
taskInfo['frames_num'] = framesNum
taskInfo['wdir'] = os.getcwd()
taskInfo['environment'] = dict()
taskInfo['environment']['PDG_DIR'] = os.getcwd()
parser.setTaskInfo(taskInfo)

arguments = []
for i in range(3, len(sys.argv)):
    arguments.append(sys.argv[i])

process = subprocess.Popen(arguments, shell=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
process.stdin.close()

stdout = process.stdout
stderr = process.stderr


def printMuted(i_str):
    sys.stdout.write('\033[1;30m')
    sys.stdout.write(i_str)
    sys.stdout.write('\033[0m')
    sys.stdout.flush()

Parser_Error = False
Parser_BadResult = False
Parser_Warning = False
Parser_Activity = None
Parser_Resources = None
Parser_Report = None

resources = ''' {"host_resources":{
    "cpu_num":4, "cpu_mhz":2790, "cpu_loadavg":[4,6,5],
    "cpu_user":9, "cpu_nice":0, "cpu_system":4, "cpu_idle":84, "cpu_iowait":0, "cpu_irq":0, "cpu_softirq":0,
    "mem_total_mb":7853, "mem_free_mb":4885, "mem_cached_mb":2724, "mem_buffers_mb":475,
    "swap_total_mb":7812, "swap_used_mb":103,
    "hdd_total_gb":34, "hdd_free_gb":11, "hdd_rd_kbsec":0, "hdd_wr_kbsec":47, "hdd_busy":0,
    "net_recv_kbsec":0, "net_send_kbsec":0,
    "logged_in_users":["foo","bar"]
}}'''
resources = json.loads(resources)

while True:
    stdout.flush()
    data = stdout.readline()
    if data is None:
        break
    if len(data) < 1:
        break

    printMuted(cgruutils.toStr(data))

    args = dict()
    args['mode'] = 'RUN'
    args['pid'] = process.pid
    args['data'] = data
    args['resources'] = json.dumps(resources)

    parser.parse(args)

    info = 'Parse:'
    info += ' %d%%: %d frame %d%%;' % (parser.percent, parser.frame, parser.percentframe)

    if len(parser.activity):
        info += ' Activity: %s;' % parser.activity
        Parser_Activity = parser.activity

    if len(parser.resources):
        info += ' Resources: %s;' % parser.resources
        Parser_Resources = parser.resources

    if len(parser.report):
        info += ' Report: %s;' % parser.report
        Parser_Report = parser.report

    if parser.warning:
        info += '\nPARSER WARNING'
        Parser_Warning = True

    if parser.error:
        info += '\nPARSER ERROR'
        Parser_Error = True

    if parser.badresult:
        info += '\nPARSER BAD RESULT'
        Parser_BadResult = True

    sys.stdout.write('%s\n' % info)
    sys.stdout.flush()

sys.stdout.flush()

print('\nResult:')

if len(parser.files_all):
    print('Files:')
    for afile in parser.files_all:
        print(afile)

StrErr = cgruutils.toStr(stderr.read()).replace('\r', '')
if len(StrErr):
    print('\nSTDERR:')
    print(StrErr)

if Parser_Activity:
    print('Activity: ' + Parser_Activity)

if Parser_Resources:
    print('Resources: ' + Parser_Resources)

if Parser_Report:
    print('Report: ' + Parser_Report)

if Parser_Warning:
    print('PARSER WARNING')

if Parser_Error:
    print('PARSER ERROR')

if Parser_BadResult:
    print('PARSER BAD RESULT')

if not Parser_Warning and not Parser_Error and not Parser_BadResult:
    print('\nSUCCESS')
