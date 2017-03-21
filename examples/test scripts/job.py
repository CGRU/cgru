#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import random
import sys
import time

import af
import afcommon

import services.service

from optparse import OptionParser

parser = OptionParser(usage="usage: %prog [Options]", version="%prog 1.0")
parser.add_option(      '--name',         dest='jobname',      type='string', default='', help='job name')
parser.add_option('-u', '--user',         dest='user',         type='string', default='', help='job user name')
parser.add_option('-l', '--labels',       dest='labels',       type='string', default='', help='blocks names (labels)')
parser.add_option(      '--services',     dest='services',     type='string', default=None, help='blocks types (services)')
parser.add_option('-t', '--time',         dest='timesec',      type='float',  default=2,  help='time per frame in seconds')
parser.add_option('-r', '--randtime',     dest='randtime',     type='float',  default=2,  help='random time per frame in seconds')
parser.add_option('-b', '--numblocks',    dest='numblocks',    type='int',    default=1,  help='number of blocks')
parser.add_option('-n', '--numtasks',     dest='numtasks',     type='int',    default=10, help='number of tasks')
parser.add_option(      '--frames',       dest='frames',       type='string', default='', help='frames "1/20/2/3,1/20/2/3"')
parser.add_option('-i', '--increment',    dest='increment',    type='int',    default=1,  help='tasks "frame increment" parameter')
parser.add_option('-p', '--pertask',      dest='pertask',      type='int',    default=1,  help='number of tasks per task')
parser.add_option('-m', '--maxtime',      dest='maxtime',      type='int',    default=0,  help='tasks maximum run time in seconds')
parser.add_option(      '--pkp',          dest='pkp',          type='int',    default=1,  help='Parser key percentage')
parser.add_option(      '--send',         dest='sendjob',      type='int',    default=1,  help='send job')
parser.add_option('-w', '--waittime',     dest='waittime',     type='int',    default=0,  help='set job to wait to start time')
parser.add_option('-c', '--capacity',     dest='capacity',     type='int',    default=0,  help='tasks capacity')
parser.add_option(      '--capmin',       dest='capmin',       type='int',    default=-1, help='tasks variable capacity coeff min')
parser.add_option(      '--capmax',       dest='capmax',       type='int',    default=-1, help='tasks variable capacity coeff max')
parser.add_option('-f', '--filesout',     dest='filesout',     type='string', default=None, help='Tasks out file')
parser.add_option(      '--filemin',      dest='filemin',      type='int',    default=-1, help='tasks output file size min')
parser.add_option(      '--filemax',      dest='filemax',      type='int',    default=-1, help='tasks output file size max')
parser.add_option(      '--mhmin',        dest='mhmin',        type='int',    default=-1, help='multi host tasks min hosts')
parser.add_option(      '--mhmax',        dest='mhmax',        type='int',    default=-1, help='multi host tasks max hosts')
parser.add_option(      '--mhwaitmax',    dest='mhwaitmax',    type='int',    default=0,  help='multi host tasks max hosts wait time seconds')
parser.add_option(      '--mhwaitsrv',    dest='mhwaitsrv',    type='int',    default=0,  help='multi host tasks service start wait time seconds')
parser.add_option(      '--mhsame',       dest='mhsame',       type='int',    default=0,  help='multi host tasks same host slave and master')
parser.add_option(      '--mhignorelost', dest='mhignorelost', type='int',    default=0,  help='multi host mosater will ignore slave lost')
parser.add_option(      '--mhservice',    dest='mhservice',    type='str',    default='', help='multi host tasks service command')
parser.add_option(      '--cmdpre',       dest='cmdpre',       type='string', default='', help='job pre command')
parser.add_option(      '--cmdpost',      dest='cmdpost',      type='string', default='', help='job post command')
parser.add_option(      '--parser',       dest='parser',       type='string', default=None, help='parser type, default if not set')
parser.add_option(      '--env',          dest='environment',  type='string', default="CG_VAR=somevalue", help='add an evironment')
parser.add_option(      '--folder',       dest='folder',       type='string', default=None, help='add a folder')
parser.add_option(      '--nofolder',     dest='nofolder',     action='store_true', default=False, help='do not set any folders')
parser.add_option(      '--pools',        dest='pools',        type='string', default=None, help='Set job render pools [/local/blender:90,/local/natron:10].')
parser.add_option(      '--seq',          dest='sequential',   type='int',    default=None, help='Sequential running')
parser.add_option(      '--ppa',          dest='ppapproval',   action='store_true', default=False, help='Preview pending approval')
parser.add_option('-e', '--exitstatus',   dest='exitstatus',   type='int',    default=0,  help='good exit status')
parser.add_option('-v', '--verbose',      dest='verbose',      type='int',    default=0,  help='tasks verbose level')
parser.add_option('-x', '--xcopy',        dest='xcopy',        type='int',    default=1,  help='number of copies to send')
parser.add_option(      '--sub',          dest='subdep',       action='store_true', default=False, help='sub task dependence')
parser.add_option('-s', '--stringtype',   dest='stringtype',   action='store_true', default=False, help='generate not numeric blocks')
parser.add_option('-o', '--output',       dest='output',       action='store_true', default=False, help='output job information')
parser.add_option(      '--pause',        dest='pause',        action='store_true', default=False, help='start job paused')

Options, args = parser.parse_args()

jobname     = Options.jobname
timesec     = Options.timesec
randtime    = Options.randtime
pkp         = Options.pkp
numblocks   = Options.numblocks
numtasks    = Options.numtasks
increment   = Options.increment
verbose     = Options.verbose
xcopy       = Options.xcopy
frames      = Options.frames.split(',')

if Options.frames != '':
    numblocks = len(frames)

if xcopy < 1:
    xcopy = 1

if jobname == '':
    jobname = '_empty_'

job = af.Job(jobname)
job.setDescription('afanasy test - empty tasks')

# Set job folder:
if Options.folder is not None:
    job.setFolder('folder', Options.folder)
if not Options.nofolder:
    job.setFolder('pwd', os.getcwd())

if Options.pools is not None:
    pools = dict()
    for pool in Options.pools.split(','):
        pool = pool.split(':')
        pools[pool[0]] = int(pool[1])
    job.setPools( pools)

blocknames = []
if Options.labels != '':
    blocknames = Options.labels.split(':')
else:
    blocknames.append('block')

blocktypes = []
if Options.services is not None:
    blocktypes = Options.services.split(':')
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

    if Options.parser is not None:
        block.setParser(Options.parser)
    else:
        block.setParser('generic')

    if b > 0:
        job.blocks[b - 1].setTasksDependMask(blockname)
        if Options.subdep:
            job.blocks[b].setDependSubTask()

    if Options.maxtime:
        block.setTasksMaxRunTime(Options.maxtime)

    if Options.capacity != 0:
        block.setCapacity(Options.capacity)

    if Options.sequential != None:
        block.setSequential( Options.sequential)

    if Options.ppapproval:
        job.setPPApproval()

    if Options.environment:
        for env in Options.environment.split(';'):
            val = env.split('=')
            block.setEnv( val[0], val[1])

    str_capacity = ''
    if Options.capmin != -1 or Options.capmax != -1:
        block.setVariableCapacity(Options.capmin, Options.capmax)
        str_capacity = ' -c ' + services.service.str_capacity

    if Options.filemin != -1 or Options.filemax != -1:
        block.setFileSizeCheck(Options.filemin, Options.filemax)

    str_hosts = ''
    if Options.mhmin != -1 or Options.mhmax != -1:
        block.setMultiHost(
            Options.mhmin, Options.mhmax, Options.mhwaitmax,
            Options.mhsame, Options.mhservice, Options.mhwaitsrv
        )
        if Options.mhignorelost:
            block.setSlaveLostIgnore()
        str_hosts = ' ' + services.service.str_hosts

    negative_pertask = False
    if Options.frames != '':
        fr = frames[b].split('/')
        if int(fr[2]) < 0:
            negative_pertask = True

    if not Options.stringtype and not negative_pertask:
        cmd = 'task.py'
        cmd = "\"%s\"" % os.path.join(os.getcwd(), cmd)
        cmd = "%s %s" % (os.getenv('CGRU_PYTHONEXE','python'), cmd)
        cmd += ' --exitstatus %d ' % Options.exitstatus

        if Options.filesout:
            cmd += ' --filesout "%s"' % Options.filesout
            block.skipExistingFiles()
            block.checkRenderedFiles( 100)
            block.setFiles(Options.filesout.split(';'))

        cmd += '%(str_capacity)s%(str_hosts)s -s @#@ -e @#@ ' \
               '-i %(increment)d -t %(timesec)g -r %(randtime)g --pkp %(pkp)d ' \
               '-v %(verbose)d @####@ @#####@ @#####@ @#####@' % vars()

        block.setCommand(cmd, False)

        if Options.frames != '':
            fr = frames[b].split('/')
            block.setNumeric(int(fr[0]), int(fr[1]), int(fr[2]), int(fr[3]))
        else:
            block.setNumeric(1, numtasks, Options.pertask, increment)

    else:
        cmd = 'task.py%(str_capacity)s @#@ -v %(verbose)d' % vars(),
        cmd = "%s %s" % (os.getenv('CGRU_PYTHONEXE','python'), cmd)
        block.setCommand( cmd, False)

        block.setTasksName('task @#@')

        if Options.filesout:
            block.setFiles(Options.filesout.split(';'))

        if Options.frames != '':
            fr = frames[b].split('/')
            block.setFramesPerTask(int(fr[2]))
            numtasks = int(fr[1]) - int(fr[0]) + 1
        for t in range(numtasks):
            timesec_task = timesec + randtime * random.random()
            task = af.Task('#' + str(t))
            task.setCommand('-s %(t)d -e %(t)d -t %(timesec_task)g' % vars())

            if Options.filesout:
                task.setFiles(['%04d' % t])

            block.tasks.append(task)

if Options.cmdpre != '':
    job.setCmdPre(Options.cmdpre)
if Options.cmdpost != '':
    job.setCmdPost(Options.cmdpost)

if Options.waittime:
    job.setWaitTime(int(time.time()) + Options.waittime)

if Options.user != '':
    job.setUserName(Options.user)

if Options.pause:
    job.offLine()

if Options.output:
    job.output(1)

job.setNeedOS('')

exit_status = 0
if Options.sendjob:
    for x in range(xcopy):
        status, data = job.send(verbose)
        if not status:
            print('Error: Job was not sent.')
            exit_status = 1
            break
        if verbose:
            print(json.dumps(data))

sys.exit(exit_status)
