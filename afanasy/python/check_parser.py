#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

while True:
    stdout.flush()
    data = stdout.readline()
    if data is None:
        break
    if len(data) < 1:
        break

    printMuted(cgruutils.toStr(data))

    parser.parse(data, 'mode', 0)

    info = 'Parse:'
    info += ' %d%%: %d frame %d%%;' % (parser.percent, parser.frame, parser.percentframe)

    if len(parser.activity):
        info += ' Activity: %s;' % parser.activity

    if len(parser.report):
        info += ' Report: %s;' % parser.report

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

if Parser_Warning:
    print('\nPARSER WARNING')

if Parser_Error:
    print('\nPARSER ERROR')

if Parser_BadResult:
    print('\nPARSER BAD RESULT')

if not Parser_Warning and not Parser_Error and not Parser_BadResult:
    print('\nSUCCESS')

StrErr = cgruutils.toStr(stderr.read()).replace('\r', '')
if len(StrErr):
    print('\nSTDERR:')
    print(StrErr)

