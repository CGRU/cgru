#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import random
import time

import af

import services.service

from optparse import OptionParser
parser = OptionParser(usage="usage: %prog [options]", version="%prog 1.0")
parser.add_option(      '--name',         dest='jobname',      type='string', default='', help='job name')
parser.add_option('-u', '--user',         dest='user',         type='string', default='', help='job user name')
parser.add_option('-l', '--labels',       dest='labels',       type='string', default='', help='blocks names (labels)')
parser.add_option(      '--services',     dest='services',     type='string', default='', help='blocks types (services)')
parser.add_option('-t', '--time',         dest='timesec',      type='float',  default=2,  help='time per frame in seconds')
parser.add_option('-r', '--randtime',     dest='randtime',     type='float',  default=2,  help='random time per frame in seconds')
parser.add_option('-b', '--numblocks',    dest='numblocks',    type='int',    default=1,  help='number of blocks')
parser.add_option('-n', '--numtasks',     dest='numtasks',     type='int',    default=10, help='number of tasks')
parser.add_option('-f', '--frames',       dest='frames',       type='string', default='', help='frames "1/20/2/3,1/20/2/3"')
parser.add_option('-i', '--increment',    dest='increment',    type='int',    default=1,  help='tasks "frame increment" parameter')
parser.add_option('-p', '--pertask',      dest='pertask',      type='int',    default=1,  help='number of tasks per task')
parser.add_option('-m', '--maxtime',      dest='maxtime',      type='int',    default=0,  help='tasks maximum run time in seconds')
parser.add_option(      '--send',         dest='sendjob',      type='int',    default=1,  help='send job')
parser.add_option('-w', '--waittime',     dest='waittime',     type='int',    default=0,  help='set job to wait to start time')
parser.add_option('-c', '--capacity',     dest='capacity',     type='int',    default=0,  help='tasks capacity')
parser.add_option(      '--capmin',       dest='capmin',       type='int',    default=-1, help='tasks variable capacity coeff min')
parser.add_option(      '--capmax',       dest='capmax',       type='int',    default=-1, help='tasks variable capacity coeff max')
parser.add_option(      '--filemin',      dest='filemin',      type='int',    default=-1, help='tasks output file size min')
parser.add_option(      '--filemax',      dest='filemax',      type='int',    default=-1, help='tasks output file size max')
parser.add_option(      '--mhmin',        dest='mhmin',        type='int',    default=-1, help='multi host tasks min hosts')
parser.add_option(      '--mhmax',        dest='mhmax',        type='int',    default=-1, help='multi host tasks max hosts')
parser.add_option(      '--mhwaitmax',    dest='mhwaitmax',    type='int',    default=0,  help='multi host tasks max hosts wait time seconds')
parser.add_option(      '--mhwaitsrv',    dest='mhwaitsrv',    type='int',    default=0,  help='multi host tasks service start wait time seconds')
parser.add_option(      '--mhsame',       dest='mhsame',       type='int',    default=0,  help='multi host tasks same host slave and master')
parser.add_option(      '--mhservice',    dest='mhservice',    type='str',    default='', help='multi host tasks service command')
parser.add_option(      '--cmdpre',       dest='cmdpre',       type='string', default='', help='job pre command')
parser.add_option(      '--cmdpost',      dest='cmdpost',      type='string', default='', help='job post command')
parser.add_option(      '--parser',       dest='parser',       type='string', default='', help='parser type, default if not set')
parser.add_option('-v', '--verbose',      dest='verbose',      type='int',    default=0,  help='tasks verbose level')
parser.add_option('-x', '--xcopy',        dest='xcopy',        type='int',    default=1,  help='number of copies to send')
parser.add_option(      '--sub',          dest='subdep',       action='store_true', default=False, help='sub task dependence')
parser.add_option(      '--nonseq',       dest='nonseq',       action='store_true', default=False, help='task non-sequential running')
parser.add_option('-s', '--stringtype',   dest='stringtype',   action='store_true', default=False, help='generate not numeric blocks')
parser.add_option('-o', '--output',       dest='output',       action='store_true', default=False, help='output job information')
parser.add_option(      '--pause',        dest='pause',        action='store_true', default=False, help='start job paused')
(options, args) = parser.parse_args()
jobname     = options.jobname
timesec     = options.timesec
randtime    = options.randtime
numblocks   = options.numblocks
numtasks    = options.numtasks
increment   = options.increment
verbose     = options.verbose
xcopy       = options.xcopy
frames      = options.frames.split(',')

if options.frames != '': numblocks = len(frames)

if xcopy < 1: xcopy = 1

if jobname == '': jobname = '_empty_'
job = af.Job( jobname)
job.setDescription('afanasy test - empty tasks')

blocknames = []
if options.labels != '': blocknames = options.labels.split(':')
else: blocknames.append('block')

blocktypes = []
if options.services != '': blocktypes = options.services.split(':')
else: blocktypes.append('generic')

if numblocks < len( blocknames): numblocks = len( blocknames)
if numblocks < len( blocktypes): numblocks = len( blocktypes)

for b in range( numblocks):
   blockname = 'block'
   blocktype = 'generic'

   if len( blocknames) > b: blockname = blocknames[b]
   else: blockname = blocknames[len( blocknames) - 1] + str(b)

   if len( blocktypes) > b: blocktype = blocktypes[b]
   else: blocktype = blocktypes[len( blocktypes) - 1]

   block = af.Block( blockname, blocktype)
   job.blocks.append( block)

   if options.parser != '': block.setParser( options.parser)

   if b > 0:
      job.blocks[b-1].setTasksDependMask( blockname)
      if options.subdep: job.blocks[b].setDependSubTask()

   if options.maxtime: block.setTasksMaxRunTime( options.maxtime)

   if options.capacity != 0 : block.setCapacity( options.capacity)

   if options.nonseq: block.setNonSequential()

   str_capacity = ''
   if options.capmin != -1 or options.capmax != -1:
      block.setVariableCapacity( options.capmin, options.capmax)
      str_capacity = ' -c ' + services.service.str_capacity

   if options.filemin != -1 or options.filemax != -1: block.setFileSizeCheck( options.filemin, options.filemax)

   str_hosts = ''
   if options.mhmin != -1 or options.mhmax != -1:
      block.setMultiHost( options.mhmin, options.mhmax, options.mhwaitmax, options.mhsame, options.mhservice, options.mhwaitsrv)
      str_hosts = ' ' + services.service.str_hosts

   negative_pertask = False
   if options.frames != '':
      fr = frames[b].split('/')
      if int(fr[2]) < 0: negative_pertask = True

   if not options.stringtype and not negative_pertask:
      cmd = 'task.py'
      cmd = os.path.join( os.getcwd(), cmd)
      cmd = 'python "%s"' % cmd
      cmd += '%(str_capacity)s%(str_hosts)s -s @#@ -e @#@ -i %(increment)d -t %(timesec)g -r %(randtime)g -v %(verbose)d @####@ @#####@ @#####@ @#####@' % vars()
      block.setCommand( cmd, False)
      if options.frames != '':
         fr = frames[b].split('/')
         block.setNumeric( int(fr[0]), int(fr[1]), int(fr[2]), int(fr[3]))
      else:
         block.setNumeric( 1, numtasks, options.pertask, increment)
      if options.pertask > 1:
         block.setFiles('file_a.@#@.@###@-file_a.@#@.@###@;file_b.@#@.@###@-file_b.@#@.@###@')
      else:
         block.setFiles('file_a.@#@.@####@;file_b.@#@.@####@')
   else:
      block.setCommand('python task.py%(str_capacity)s @#@ -v %(verbose)d' % vars(), False)
      block.setTasksName('task @#@')
      block.setFiles('file_a.@#@;file_b.@#@')
      if options.frames != '':
         fr = frames[b].split('/')
         block.setFramesPerTask( int(fr[2]))
         numtasks = int(fr[1]) - int(fr[0]) + 1
      for t in range( numtasks):
         timesec_task = timesec + randtime * random.random()
         task = af.Task('#' + str(t))
         task.setCommand('-s %(t)d -e %(t)d -t %(timesec_task)g' % vars());
         task.setFiles('%04d' % t)
         block.tasks.append( task)

if options.cmdpre  != '':
   job.setCmdPre( options.cmdpre)
if options.cmdpost != '':
   job.setCmdPost( options.cmdpost)

if options.waittime:
   job.setWaitTime( int(time.time()) + options.waittime)

if options.user != '':
   job.setUserName( options.user)

if options.pause:
   job.offLine()

if options.output:
   job.output( 1)

job.setNeedOS('')

exit_status = 0
if options.sendjob:
   for x in range(xcopy):
      if job.send( verbose) == False:
         print('Error: Job was not sent.')
         exit_status = 1
         break

sys.exit( exit_status)
