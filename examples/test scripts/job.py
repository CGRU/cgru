#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''
/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

This is testing job.

'''
import argparse
import json
import os
import random
import sys
import time

import af
import afcommon

import services.service

parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter, epilog="""
Examples:
 * Job with sub-task dependence:
./job.sh --sub --name 'Sub Task Dependence' --labels 'render,simulation' --frames '1/30/1/1,1/30/30/1'
""")
parser.add_argument(      '--name',         dest='jobname',      type=str,   default='', help='job name')
parser.add_argument('-u', '--user',         dest='user',         type=str,   default='', help='job user name')
parser.add_argument('-l', '--labels',       dest='labels',       type=str,   default='', help='blocks names (labels)')
parser.add_argument(      '--services',     dest='services',     type=str,   default=None, help='blocks types (services)')
parser.add_argument('-t', '--time',         dest='timesec',      type=float, default=2,  help='time per frame in seconds')
parser.add_argument('-r', '--randtime',     dest='randtime',     type=float, default=2,  help='random time per frame in seconds')
parser.add_argument('-b', '--numblocks',    dest='numblocks',    type=int,   default=1,  help='number of blocks')
parser.add_argument('-n', '--numtasks',     dest='numtasks',     type=int,   default=10, help='number of tasks')
parser.add_argument(      '--frames',       dest='frames',       type=str,   default='', help='frames "1/20/2/3,1/20/2/3"')
parser.add_argument('-i', '--increment',    dest='increment',    type=int,   default=1,  help='tasks "frame increment" parameter')
parser.add_argument('-p', '--pertask',      dest='pertask',      type=int,   default=1,  help='number of tasks per task')
parser.add_argument('-m', '--maxtime',      dest='maxtime',      type=int,   default=0,  help='tasks maximum run time in seconds')
parser.add_argument(      '--timeout',      dest='timeout',      type=int,   default=0,  help='tasks progress change timeout in seconds')
parser.add_argument(      '--pkp',          dest='pkp',          type=int,   default=1,  help='Parser key percentage')
parser.add_argument(      '--send',         dest='sendjob',      type=int,   default=1,  help='send job')
parser.add_argument('-w', '--waittime',     dest='waittime',     type=int,   default=0,  help='set job to wait to start time')
parser.add_argument(      '--os',           dest='os',           type=str,   default=None, help='OS needed')
parser.add_argument('-c', '--capacity',     dest='capacity',     type=int,   default=0,  help='tasks capacity')
parser.add_argument(      '--capmin',       dest='capmin',       type=int,   default=-1, help='tasks variable capacity coeff min')
parser.add_argument(      '--capmax',       dest='capmax',       type=int,   default=-1, help='tasks variable capacity coeff max')
parser.add_argument('-f', '--filesout',     dest='filesout',     type=str,   default=None, help='Tasks out file [render/img.%%04d.jpg]')
parser.add_argument(      '--skipexist',    dest='skipexist',    action='store_true', default=False, help='Skip existing files.')
parser.add_argument(      '--imgres',       dest='imgres',       type=str,   default=None, help='Images resolution [640x480]')
parser.add_argument(      '--filemin',      dest='filemin',      type=int,   default=-1, help='tasks output file size min')
parser.add_argument(      '--filemax',      dest='filemax',      type=int,   default=-1, help='tasks output file size max')
parser.add_argument(      '--stdoutfile',   dest='stdoutfile',   type=str,   default=None, help='Read tasks stdout from file')
parser.add_argument(      '--mhmin',        dest='mhmin',        type=int,   default=-1, help='multi host tasks min hosts')
parser.add_argument(      '--mhmax',        dest='mhmax',        type=int,   default=-1, help='multi host tasks max hosts')
parser.add_argument(      '--mhwaitmax',    dest='mhwaitmax',    type=int,   default=0,  help='multi host tasks max hosts wait time seconds')
parser.add_argument(      '--mhwaitsrv',    dest='mhwaitsrv',    type=int,   default=0,  help='multi host tasks service start wait time seconds')
parser.add_argument(      '--mhsame',       dest='mhsame',       type=int,   default=0,  help='multi host tasks same host slave and master')
parser.add_argument(      '--mhignorelost', dest='mhignorelost', type=int,   default=0,  help='multi host mosater will ignore slave lost')
parser.add_argument(      '--mhservice',    dest='mhservice',    type=str,   default='', help='multi host tasks service command')
parser.add_argument(      '--cmd',          dest='cmd',          type=str,   default=None, help='Tasks command')
parser.add_argument(      '--cmdpre',       dest='cmdpre',       type=str,   default='', help='job pre command')
parser.add_argument(      '--cmdpost',      dest='cmdpost',      type=str,   default='', help='job post command')
parser.add_argument(      '--parser',       dest='parser',       type=str,   default=None, help='parser type, default if not set')
parser.add_argument(      '--env',          dest='environment',  type=str,   default='CG_VAR=somevalue', help='add an evironment, example: "CG_VAR=somevalue"')
parser.add_argument(      '--tickets',      dest='tickets',      type=str,   default='GPU:1,NET:100', help='add tickets, example: "MEM:32,NET:100"')
parser.add_argument(      '--folder',       dest='folder',       type=str,   default=None, help='add a folder')
parser.add_argument(      '--pools',        dest='pools',        type=str,   default=None, help='Set job render pools [/local/blender:90,/local/natron:10].')
parser.add_argument(      '--branch',       dest='branch',       type=str,   default=None, help='Set job branch.')
parser.add_argument(      '--try',          dest='trytasks',     type=str,   default=None, help='Try tasks "0:3,0:5"')
parser.add_argument(      '--seq',          dest='sequential',   type=int,   default=None, help='Sequential running')
parser.add_argument('-e', '--exitstatus',   dest='exitstatus',   type=int,   default=0,  help='good exit status')
parser.add_argument('-v', '--verbose',      dest='verbose',      type=int,   default=0,  help='tasks verbose level')
parser.add_argument('-x', '--xcopy',        dest='xcopy',        type=int,   default=1,  help='number of copies to send')
parser.add_argument(      '--ppa',          dest='ppapproval',   action='store_true', default=False, help='Preview pending approval')
parser.add_argument(      '--nofolder',     dest='nofolder',     action='store_true', default=False, help='do not set any folders')
parser.add_argument(      '--sub',          dest='subdep',       action='store_true', default=False, help='sub task dependence')
parser.add_argument(      '--depblock',     dest='depblock',     action='store_true', default=False, help='Blocks will be depend by block depend mask. If not set, blocks will bepend by tasks.')
parser.add_argument('-s', '--stringtype',   dest='stringtype',   action='store_true', default=False, help='generate not numeric blocks')
parser.add_argument('-o', '--output',       dest='output',       action='store_true', default=False, help='output job information')
parser.add_argument(      '--pause',        dest='pause',        action='store_true', default=False, help='start job paused')

Args = parser.parse_args()

jobname     = Args.jobname
timesec     = Args.timesec
randtime    = Args.randtime
pkp         = Args.pkp
numblocks   = Args.numblocks
numtasks    = Args.numtasks
increment   = Args.increment
verbose     = Args.verbose
xcopy       = Args.xcopy
frames      = Args.frames.split(',')

if Args.frames != '':
    numblocks = len(frames)

if xcopy < 1:
    xcopy = 1

if jobname == '':
    jobname = '_empty_'

job = af.Job(jobname)
job.setDescription('afanasy test - empty tasks')

# Set job folder:
if Args.folder is not None:
    job.setFolder('folder', Args.folder)
if not Args.nofolder:
    job.setFolder('pwd', os.getcwd())

if Args.pools is not None:
    pools = dict()
    for pool in Args.pools.split(','):
        pool = pool.split(':')
        pools[pool[0]] = int(pool[1])
    job.setPools( pools)

if Args.branch is not None:
    job.setBranch( Args.branch)
else:
    job.setBranch( os.getcwd())

if Args.trytasks:
    for pair in Args.trytasks.split(','):
        bt = pair.split(':')
        if len(bt) == 2:
            job.tryTask(int(bt[0]), int(bt[1]))

blocknames = []
if Args.labels != '':
    blocknames = Args.labels.split(',')
else:
    blocknames.append('block')

blocktypes = []
if Args.services is not None:
    blocktypes = Args.services.split(',')
else:
    blocktypes.append('test')

if numblocks < len(blocknames):
    numblocks = len(blocknames)

if numblocks < len(blocktypes):
    numblocks = len(blocktypes)

for b in range(numblocks):
    blockname = 'block'
    blocktype = 'test'

    if len(blocknames) > b:
        blockname = blocknames[b]
    else:
        blockname = blocknames[len(blocknames) - 1] + str(b)

    if len(blocktypes) > b:
        blocktype = blocktypes[b]
    else:
        blocktype = blocktypes[len(blocktypes) - 1]

    block = af.Block(blockname, blocktype)
    job.blocks.append(block)

    if Args.parser is not None:
        block.setParser(Args.parser)

    if b > 0:
        if Args.depblock:
            job.blocks[b - 1].setDependMask(blockname)
        else:
            job.blocks[b - 1].setTasksDependMask(blockname)
            if Args.subdep:
                job.blocks[b].setDependSubTask()

    if Args.maxtime:
        block.setTasksMaxRunTime(Args.maxtime)

    if Args.timesec:
        block.setTaskProgressChangeTimeout(Args.timeout)

    if Args.capacity != 0:
        block.setCapacity(Args.capacity)

    if Args.sequential != None:
        block.setSequential( Args.sequential)

    if Args.ppapproval:
        job.setPPApproval()

    if Args.environment:
        for env in Args.environment.split(';'):
            vals = env.split('=')
            if len(vals) == 2:
                block.setEnv(vals[0], vals[1])
            else:
                'Warning: Invalid environment: "%s"' % Args.environment

    if Args.tickets:
        for env in Args.tickets.split(','):
            vals = env.split(':')
            if len(vals) == 2:
                block.addTicket(vals[0], int(vals[1]))
            else:
                'Warning: Invalid tickets: "%s"' % Args.environment

    if Args.filemin != -1 or Args.filemax != -1:
        block.skipExistingFiles(Args.filemin, Args.filemax)
        block.checkRenderedFiles(Args.filemin, Args.filemax)

    negative_pertask = False
    if Args.frames != '':
        fr = frames[b].split('/')
        if int(fr[2]) < 0:
            negative_pertask = True

    cmd = 'task.py'
    cmd = "\"%s\"" % os.path.join(os.getcwd(), cmd)
    cmd = "%s %s" % (os.getenv('CGRU_PYTHONEXE','python3'), cmd)
    cmd += ' --exitstatus %d ' % Args.exitstatus

    if Args.capmin != -1 or Args.capmax != -1:
        block.setVariableCapacity(Args.capmin, Args.capmax)
        cmd += ' -c ' + services.service.str_capacity

    if Args.mhmin != -1 or Args.mhmax != -1:
        block.setMultiHost(
            Args.mhmin, Args.mhmax, Args.mhwaitmax,
            Args.mhsame, Args.mhservice, Args.mhwaitsrv
        )
        if Args.mhignorelost:
            block.setSlaveLostIgnore()
        cmd += ' ' + services.service.str_hosts

    if Args.filesout:
        cmd += ' --filesout "%s"' % Args.filesout
        filemin = 100
        if Args.filemin > 0:
            filemin = Args.filemin
        block.checkRenderedFiles(filemin, Args.filemax)
        if Args.skipexist:
            block.skipExistingFiles(filemin, Args.filemax)
        files = []
        for afile in Args.filesout.split(';'):
            files.append(afcommon.patternFromStdC(afile))
        block.setFiles(files)
        if Args.imgres:
            cmd += ' --imgres "%s"' % Args.imgres

    if Args.stdoutfile:
        if not os.path.isfile(Args.stdoutfile):
            print('ERROR: File "%s" does not exist.')
            sys.exit(1)
        cmd += ' --stdoutfile "%s"' % Args.stdoutfile

    if not Args.stringtype and not negative_pertask:
        cmd += ' -s @#@ -e @#@ -i %(increment)d' \
               ' -t %(timesec)g -r %(randtime)g --pkp %(pkp)d' \
               ' -v %(verbose)d @####@ @#####@ @#####@ @#####@' % vars()

        if Args.frames != '':
            fr = frames[b].split('/')
            block.setNumeric(int(fr[0]), int(fr[1]), int(fr[2]), int(fr[3]))
        else:
            block.setNumeric(1, numtasks, Args.pertask, increment)

    else:
        cmd += ' -v %(verbose)d' % vars()
        cmd += ' @#@'

        block.setTasksName('task @#@')

        if Args.frames != '':
            fr = frames[b].split('/')
            block.setFramesPerTask(int(fr[2]))
            numtasks = int(fr[1]) - int(fr[0]) + 1

        for t in range(numtasks):
            timesec_task = timesec + randtime * random.random()
            task = af.Task('#' + str(t))
            task.setCommand('-s %(t)d -e %(t)d -t %(timesec_task)g' % vars())

            if Args.filesout:
                task.setFiles(['%04d' % t])

            block.tasks.append(task)

    if Args.cmd:
        cmd = Args.cmd

    block.setCommand(cmd, False)

if Args.cmdpre != '':
    job.setCmdPre(Args.cmdpre)
if Args.cmdpost != '':
    job.setCmdPost(Args.cmdpost)

if Args.waittime:
    job.setWaitTime(int(time.time()) + Args.waittime)

if Args.user != '':
    job.setUserName(Args.user)

if Args.pause:
    job.offLine()

if Args.output:
    job.output()

if Args.os is None:
    job.setNeedOS('')
else:
    job.setNeedOS(Args.os)

exit_status = 0
if Args.sendjob:
    for x in range(xcopy):
        status, data = job.send(verbose)
        if not status:
            print('Error: Job was not sent.')
            exit_status = 1
            break
        if verbose:
            print(json.dumps(data))

sys.exit(exit_status)
