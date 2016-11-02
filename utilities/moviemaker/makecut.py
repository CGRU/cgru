#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import shutil
import signal
import re
import time

import af

from optparse import OptionParser

Extensions = ['jpg', 'png', 'dpx']
TmpFiles = 'img.%07d.jpg'

Parser = OptionParser(
    usage="%prog [options] input\n"
          "Pattern examples = \"img.####.jpg\" or \"img.%04d.jpg\".\n"
          "Type \"%prog -h\" for help", version="%prog 1.0"
)

Parser.add_option('-i', '--inputs',     dest='inputs',      type  ='string', default='RESULT/JPG',help='Inputs')
Parser.add_option('-n', '--cutname',    dest='cutname',     type  ='string', default='',          help='Cut name')
Parser.add_option('-f', '--fps',        dest='fps',         type  ='string', default='24',        help='FPS')
Parser.add_option('-r', '--resolution', dest='resolution',  type  ='string', default='1280x720',  help='Resolution: 1280x720')
Parser.add_option('-c', '--codec',      dest='codec',       type  ='string', default='h264_good', help='Codec')
Parser.add_option(      '--font',       dest='font' ,       type  ='string', default=None,        help='Annotations font')
Parser.add_option('--colorspace',       dest='colorspace',  type  ='string', default='auto',      help='Input images colorspace')
Parser.add_option('-o', '--outdir',     dest='outdir',      type  ='string', default='cut',       help='Output folder')
Parser.add_option('-u', '--afuser',     dest='afuser',      type  ='string', default='',          help='Afanasy user name')
Parser.add_option('--afservice',        dest='afservice',   type  ='string', default='movgen',    help='Afanasy service type')
Parser.add_option('--afcapacity',       dest='afcapacity',  type  ='int',    default=100,         help='Afanasy tasks capacity')
Parser.add_option('--afmaxtasks',       dest='afmaxtasks',  type  ='int',    default=-1,          help='Afanasy max running tasks')
Parser.add_option('--afperhost',        dest='afperhost',   type  ='int',    default=-1,          help='Afanasy max tasks per host')
Parser.add_option('--afmaxruntime',     dest='afmaxruntime',type  ='int',    default=120,         help='Afanasy max tasks run time')
Parser.add_option('-p', '--afpertask',  dest='afpertask',   type  ='int',    default=100,         help='Afanasy frames per task')
Parser.add_option('-t', '--testonly',   dest='testonly',    action='store_true', default=False,   help='Test input only')
Parser.add_option('-V', '--verbose',    dest='verbose',     action='store_true', default=False,   help='Verbose')


def errExit(i_msg):
    print('{"error":"%s"},' % i_msg)
    print('{"status":"error"}]}')
    sys.exit(1)


def interrupt(signum, frame):
    errExit('Interrupt received')


signal.signal(signal.SIGTERM, interrupt)
signal.signal(signal.SIGABRT, interrupt)
signal.signal(signal.SIGINT, interrupt)

print('{"cut":[')

(Options, args) = Parser.parse_args()

if len(args) < 1:
    errExit('Not enough arguments provided.')

Inputs = Options.inputs.split(',')
Shots = args
CutName = Options.cutname

if os.path.isfile(args[-1]):
    Shots = args[-1]
    if CutName == '':
        CutName = os.path.basename(Shots)
    with open(Shots) as f:
        Shots = f.readlines()

if len(Shots) < 2:
    errExit('Less than 2 shots provided.')

if CutName == '':
    CutName = os.path.basename(os.path.dirname(Shots[0]))

OutDir = Options.outdir + '/' + CutName
OutDir = os.path.normpath(OutDir)

movie_name = os.path.basename(CutName) + time.strftime('_%y-%m-%d_%H-%M-%S')
movie_name = os.path.join(Options.outdir, movie_name)

commands = []
task_names = []
cmd_prefix = os.path.join(os.path.dirname(sys.argv[0]), 'makeframe.py')
cmd_prefix = 'python "%s"' % os.path.normpath(cmd_prefix)
cmd_prefix += ' --mkdir'
cmd_prefix += ' -t "dailies"'
cmd_prefix += ' -r %s' % Options.resolution
cmd_prefix += ' -d "%s"' % time.strftime('%y-%m-%d')
cmd_prefix += ' --colorspace "%s"' % Options.colorspace
if Options.font:
    cmd_prefix += ' --font "%s"' % Options.font

file_counter = 0

for shot in Shots:
    shot = shot.strip()
    if len(shot) == 0:
        continue
    if shot[0] == '#':
        continue

    folder = None
    version = None
    name = os.path.basename(shot)

    for inp in Inputs:
        inp = os.path.join(shot, inp)
        if not os.path.isdir(inp):
            continue

        for item in os.listdir(inp):
            if item[0] in '._':
                continue
            if not os.path.isdir(os.path.join(inp, item)):
                continue
            ver = item.replace(name, '').strip('_. ')
            if version is not None:
                if version >= ver:
                    continue
            version = ver
            folder = os.path.join(inp, item)

    if folder is None:
        errExit('Input not found for: %s' % shot)

    files = []
    for item in os.listdir(folder):
        valid = False
        for ext in Extensions:
            if item[-len(ext):].lower() == ext:
                valid = True
                break
        if valid:
            files.append(os.path.join(folder, item))

    if len(files) == 0:
        errExit('No files found in folder: %s' % folder)

    files.sort()

    nums = re.findall(r'\d+',files[0])
    if len(nums) == 0:
        errExit('Can`t find numbers in %s', files[0])
    nums = nums[-1]
    sequence = files[0][:files[0].rfind(nums)]
    padding = len(nums)
    sequence += '%0' + str(padding) + 'd'
    sequence += files[0][files[0].rfind(nums)+padding:]
    frame_first = int(nums)
    nums = re.findall(r'\d+',files[-1])
    if len(nums) == 0:
        errExit('Can`t find numbers in %s', files[-1])
    frame_last = int(nums[-1])

    print('{"sequence":"%s","first":%d,"last":%d,"count":%d},' % (sequence, frame_first, frame_last, len(files)))

    f = frame_first
    while f <= frame_last:
        num_frames = len(files)

        if num_frames > Options.afpertask:        
            num_frames = Options.afpertask

        if f + num_frames > frame_last:
            num_frames = frame_last - f + 1

        cmd = cmd_prefix
        cmd += ' --project "%s"' % CutName
        cmd += ' --shot "%s"' % name
        cmd += ' --ver "%s"' % version
        cmd += ' --moviename "%s"' % os.path.basename(movie_name)
        cmd += ' --frame_input %d' % f
        cmd += ' --frame_output %d' % file_counter
        cmd += ' --frames_num %d' % num_frames
        cmd += ' "%s"' % sequence
        output = os.path.join(OutDir, TmpFiles)
        cmd += ' "%s"' % output

        commands.append(cmd)
        task_names.append('%s: %d-%d' % (os.path.basename(sequence), f, f+num_frames-1))

        f += num_frames
        file_counter += num_frames


print('{"progress":"%d sequences found"},' % len(Shots))
print('{"progress":"%d files found"},' % file_counter)

cmd_encode = os.path.join(os.path.dirname(sys.argv[0]), 'makemovie.py')
cmd_encode = 'python "%s"' % os.path.normpath(cmd_encode)
cmd_encode += ' -f %s' % Options.fps
cmd_encode += ' -c %s' % Options.codec
cmd_encode += ' "%s"' % os.path.join(OutDir, TmpFiles)
cmd_encode += ' "%s"' % movie_name


# Afanasy job creation:
job = af.Job('CUT ' + CutName)
job.setMaxRunningTasks( Options.afmaxtasks)
job.setMaxRunTasksPerHost( Options.afperhost)
if Options.afuser != '': job.setUserName(Options.afuser)

# Delete previous sequence block:
delete_name = None
if os.path.isdir(OutDir):
    delete_name = 'delete'
    block = af.Block( delete_name)
    block.setCapacity( 1)
    task = af.Task( delete_name + ' ' + os.path.basename( OutDir))
    task.setCommand('deletefiles "%s"' % OutDir)
    block.tasks.append(task)
    job.blocks.append(block)

# Convert block:
block = af.Block('convert', Options.afservice)
if delete_name: block.setDependMask( delete_name)
counter = 0
for cmd in commands:
    task = af.Task(task_names[counter])
    task.setCommand(cmd)
    block.tasks.append(task)
    counter += 1
    if Options.verbose: print(cmd)
block.setCapacity( Options.afcapacity)
block.setTasksMaxRunTime( Options.afmaxruntime)
job.blocks.append(block)

# Encode block:
block = af.Block('encode', Options.afservice)
block.setDependMask('convert')
block.setCapacity( Options.afcapacity)
task = af.Task('encode')
task.setCommand(cmd_encode)
block.tasks.append(task)
job.blocks.append(block)


if not Options.testonly:
    if not job.send():
        errExit('Can`t send job to server.')

print('{"status":"success"}]}')
