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
parser.add_option(      '--name',      dest='jobname',   type='string', default='', help='job name')
parser.add_option('-l', '--labels',    dest='labels',    type='string', default='', help='blocks names (labels)')
parser.add_option(      '--types',     dest='types',     type='string', default='', help='blocks types (services)')
parser.add_option('-t', '--time',      dest='timesec',   type='float',  default=2,  help='time per frame in seconds')
parser.add_option('-r', '--randtime',  dest='randtime',  type='float',  default=2,  help='random time per frame in seconds')
parser.add_option('-b', '--numblocks', dest='numblocks', type='int',    default=1,  help='number of blocks')
parser.add_option('-n', '--numtasks',  dest='numtasks',  type='int',    default=10, help='number of tasks')
parser.add_option('-p', '--perhost',   dest='perhost',   type='int',    default=1,  help='number of tasks per host')
parser.add_option(      '--numeric',   dest='numeric',   type='int',    default=1,  help='create numeric blocks')
parser.add_option('-v', '--verbose',   dest='verbose',   type='int',    default=0,  help='enable tasks verbose')
parser.add_option('-o', '--output',    dest='output',    type='int',    default=0,  help='output job information')
parser.add_option('-m', '--maxtime',   dest='maxtime',   type='int',    default=0,  help='tasks maximum run time in seconds')
parser.add_option('-s', '--send',      dest='sendjob',   type='int',    default=1,  help='send job')
parser.add_option('-w', '--waittime',  dest='waittime',  type='int',    default=0,  help='set job to wait to start time')
parser.add_option('-c', '--capacity',  dest='capacity',  type='int',    default=0,  help='tasks capacity')
parser.add_option(      '--capmin',    dest='capmin',    type='int',    default=-1, help='tasks variable capacity coeff min')
parser.add_option(      '--capmax',    dest='capmax',    type='int',    default=-1, help='tasks variable capacity coeff max')
parser.add_option(      '--filemin',   dest='filemin',   type='int',    default=-1, help='tasks output file size min')
parser.add_option(      '--filemax',   dest='filemax',   type='int',    default=-1, help='tasks output file size max')
parser.add_option(      '--mhmin',     dest='mhmin',     type='int',    default=-1, help='multi host tasks min hosts')
parser.add_option(      '--mhmax',     dest='mhmax',     type='int',    default=-1, help='multi host tasks max hosts')
parser.add_option(      '--mhwaitmax', dest='mhwaitmax', type='int',    default=0,  help='multi host tasks max hosts wait time seconds')
parser.add_option(      '--mhwaitsrv', dest='mhwaitsrv', type='int',    default=0,  help='multi host tasks service start wait time seconds')
parser.add_option(      '--mhsame',    dest='mhsame',    type='int',    default=0,  help='multi host tasks same host slave and master')
parser.add_option(      '--mhservice', dest='mhservice', type='int',    default=-1, help='multi host tasks service emulaton sleep seconds')
parser.add_option(      '--cmdpre',    dest='cmdpre',    type='string', default='', help='job pre command')
parser.add_option(      '--cmdpost',   dest='cmdpost',   type='string', default='', help='job post command')
parser.add_option(      '--pause',     dest='pause',     type='int',    default=0,  help='start job paused')
(options, args) = parser.parse_args()
jobname   = options.jobname
labels    = options.labels
types     = options.types
timesec   = options.timesec
randtime  = options.randtime
numblocks = options.numblocks
numtasks  = options.numtasks
perhost   = options.perhost
numeric   = options.numeric
verbose   = options.verbose
output    = options.output
maxtime   = options.maxtime
sendjob   = options.sendjob
waittime  = options.waittime
capacity  = options.capacity
cmdpre    = options.cmdpre
cmdpost   = options.cmdpost
capmin    = options.capmin
capmax    = options.capmax
filemin   = options.filemin
filemax   = options.filemax
mhmin     = options.mhmin
mhmax     = options.mhmax
mhwaitmax = options.mhwaitmax
mhwaitsrv = options.mhwaitsrv
mhsame    = options.mhsame
mhservice = options.mhservice
pause     = options.pause

if jobname == '': jobname = '_empty_'
job = af.Job( jobname)
job.setDescription('afanasy test - empty tasks')

blocknames = []
if labels != '': blocknames = labels.split(':')
else: blocknames.append('block')

blocktypes = []
if types != '': blocktypes = types.split(':')
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

   block.setParserType('generic')

   if b > 0: job.blocks[b-1].setTasksDependMask( blockname)

   if maxtime: block.setTasksMaxRunTime( maxtime)

   if capacity != 0 : block.setCapacity( capacity)

   str_capacity = ''
   if capmin != -1 or capmax != -1:
      block.setVariableCapacity( capmin, capmax)
      str_capacity = ' -c ' + services.service.str_capacity

   if filemin != -1 or filemax != -1: block.setFileSizeCheck( filemin, filemax)

   str_hosts = ''
   if mhmin != -1 or mhmax != -1:
      cmdservice = ''
      if mhservice != -1: cmdservice = 'sleep ' + str(mhservice)
      block.setMultiHost( mhmin, mhmax, mhwaitmax, mhsame, cmdservice, mhwaitsrv)
      str_hosts = ' ' + services.service.str_hosts

   if numeric:
      block.setCommand('python task.py%(str_capacity)s%(str_hosts)s -s %%1 -e %%2 -t %(timesec)g -r %(randtime)g -v %(verbose)d' % vars(), False)
      block.setNumeric( 1, numtasks, perhost)
      block.setTasksName('num %1 %2')
      block.setCommandView('view %1-%2')
   else:
      block.setCommand('python task.py%(str_capacity)s %%1 -v %(verbose)d' % vars(), False)
      block.setTasksName('task %1')
      block.setCommandView('view %1')
      for t in range( numtasks):
         timesec_task = timesec + randtime * random.random()
         task = af.Task('#' + str(t))
         task.setCommand('-s %(t)d -e %(t)d -t %(timesec_task)g' % vars());
         task.setCommandView('task.' + str(t))
         block.tasks.append( task)

if cmdpre  != '':
   job.setCmdPre( cmdpre)
if cmdpost != '':
   job.setCmdPost( cmdpost)

if waittime:
   job.setWaitTime( int(time.time()) + waittime)

if pause:
   job.offLine()

if output:
   job.output( 1)

if sendjob:
   job.send()
