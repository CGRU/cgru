#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import sys
import time

from optparse import OptionParser

Parser = OptionParser(
    usage="%prog [options]\ntype \"%prog -h\" for help",
    version="%prog 1.0"
)

Parser.add_option('-o', '--output',   dest='output',   type = 'string',     default='.rules/walk.json', help='File to save results.')
Parser.add_option('-n', '--noupdate', dest='noupdate', action='store_true', default=False,              help='Skip update upfolders.')
Parser.add_option('-V', '--verbose',  dest='verbose',  type = 'int',        default=0,                  help='Verbose mode.')
Parser.add_option('-D', '--debug',    dest='debug',    action='store_true', default=False,              help='Debug mode.')

(Options, Args) = Parser.parse_args()

Progress = 0
PrevFiles = None
CurFiles = 0
StartPath = '.'
os.umask(0000)

if len(Args):
    StartPath = Args[0]

if not os.path.isdir(StartPath):
    print('ERROR: Starting path does not exist:')
    print( StartPath)
    sys.exit(1)


def printFlush(i_msg):
    print(i_msg)
    sys.stdout.flush()


def jsonLoad(i_filename):
    if not os.path.isfile(i_filename):
        return None

    try:
        file = open(i_filename, 'r')
    except:
        printFlush(str(sys.exc_info()[1]))
        return None

    obj = None
    try:
        obj = json.load(file)
    except:
        print('ERROR: %s' % i_filename)
        printFlush(str(sys.exc_info()[1]))
        obj = None

    file.close()

    return obj


def checkDict(io_dict):
    if not 'folders' in io_dict:
        io_dict['folders'] = dict()
    if not 'files' in io_dict:
        io_dict['files'] = dict()
    num_keys = ['num_files', 'num_folders', 'size', 'num_files_total',
                'num_folders_total', 'size_total']
    for key in num_keys:
        if not key in io_dict:
            io_dict[key] = 0


def walkdir(i_path, i_subwalk, i_curdepth=0):
    global Progress
    global PrevFiles
    global CurFiles

    if Options.verbose > i_curdepth and i_subwalk:
        printFlush(i_path)

    out = dict()
    checkDict(out)

    try:
        entries = os.listdir(i_path)
    except:
        print(str(sys.exc_info()[1]))
        return None

    for entry in entries:
        # Skip result folder (.rules):
        if entry == os.path.dirname(Options.output):
            continue

        path = os.path.join(i_path, entry)

        # We are not walking in links:
        if os.path.islink(path):
            continue

        if os.path.isdir(path):
            out['num_folders'] += 1
            out['num_folders_total'] += 1

            fout = None
            if i_subwalk:
                # Recursively walk in a subfolder:
                fout = walkdir(path, True, i_curdepth + 1)
            else:
                # Load previous walk data:
                fout = jsonLoad(os.path.join(path, Options.output))

            if fout is not None:
                checkDict(fout)

                # We do not need info for each subfolder in a child folder:
                del fout['files']
                del fout['folders']
                out['folders'][entry] = fout

                out['num_folders_total'] += fout['num_folders_total']
                out['num_files_total'] += fout['num_files_total']
                out['size_total'] += fout['size_total']

        if os.path.isfile(path):
            CurFiles += 1
            out['num_files'] += 1
            out['num_files_total'] += 1
            out['size_total'] += os.path.getsize(path)
            out['size'] += os.path.getsize(path)

    # Just output progress:
    if PrevFiles:
        cur_progress = int(100.0 * CurFiles / PrevFiles)
        if cur_progress != Progress:
            Progress = cur_progress
            printFlush('PROGRESS: %d%%' % Progress)

    # Store current walk data:
    filename = os.path.join(i_path, Options.output)
    if not os.path.isdir(os.path.dirname(filename)):
        try:
            os.makedirs(os.path.dirname(filename))
        except:
            printFlush(str(sys.exc_info()[1]))
    try:
        file = open(filename, 'w')
        json.dump(out, file, indent=1)
        file.close()
    except:
        printFlush(str(sys.exc_info()[1]))

    return out


def sepTh(i_int):
    s = str(int(i_int))
    o = ''
    for i in range(0, len(s)):
        o += s[len(s) - i - 1]
        if (i + 1) % 3 == 0:
            o += ' '
    s = ''
    for i in range(0, len(o)):
        s += o[len(o) - i - 1]
    return s

# #############################################################################
time_start = time.time()
printFlush('Started at: %s' % time.ctime(time_start))

# Get old files count if any:
prev = jsonLoad(os.path.join(StartPath, Options.output))
if prev is not None:
    if 'num_files_total' in prev:
        checkDict(prev)
        PrevFiles = prev['num_files_total']

if PrevFiles:
    printFlush('Previous run: %s files, %s folders, %s bytes' % (
    sepTh(prev['num_files_total']), sepTh(prev['num_folders_total']),
    sepTh(prev['size_total'])))

# Walk in subfolders:
walk = walkdir(StartPath, True)

# Calculate difference with previous
d_files = None
d_folders = None
d_size = None
if PrevFiles:
    d_files = walk['num_files_total'] - prev['num_files_total']
    d_folders = walk['num_folders_total'] - prev['num_folders_total']
    d_size = walk['size_total'] - prev['size_total']


# Update parent folders:
if not Options.noupdate:
    curpath = os.path.abspath(StartPath)
    PrevFiles = None
    while curpath != '/':
        # Go one folder upper:
        uppath = os.path.dirname(curpath)
        if uppath == curpath:
            break
        curpath = uppath

        # Stop updating if there is not any walk data:
        if not os.path.isfile(os.path.join(curpath, Options.output)):
            break

        printFlush('Updating: %s' % curpath)
        walkdir(curpath, False)


# Output statistics:
time_finish = time.time()
print('Finished at: %s' % time.ctime(time_finish))
print('Result: %s files, %s folders, %s bytes' % (
    sepTh(walk['num_files_total']),
    sepTh(walk['num_folders_total']),
    sepTh(walk['size_total']))
)

if d_files is not None:
    print(
        'Delta: %s files, %s folders, %s bytes' %
        (sepTh(d_files), sepTh(d_folders), sepTh(d_size))
    )

sec = time_finish - time_start
hrs = int(sec / 3600)
sec -= hrs * 3600
mns = int(sec / 60)
sec -= mns * 60
msc = int(1000.0 * sec - int(sec))
sec = int(sec)
print('Run time: %02d:%02d:%02d.%03d' % (hrs, mns, sec, msc))
