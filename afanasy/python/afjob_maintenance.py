#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import af
import afcmd

from optparse import OptionParser

Parser = OptionParser(
    usage="\
    \n%prog render1 render2 ... command\
    \n%prog --mask 'regexp' command\
    \ntype \"%prog -h\" for "
    "help", version="%prog 1.  0"
)
Parser.add_option(      '--jobname',     dest='jobname',     type='string',       default=None,      help='Job name')
Parser.add_option(      '--service',     dest='service',     type='string',       default='generic', help='Service type')
Parser.add_option(      '--parser',      dest='parser',      type='string',       default='generic', help='Parser type')
Parser.add_option(      '--maxruntasks', dest='maxruntasks', type='int',          default=5,         help='Maximum running tasks')
Parser.add_option(      '--mask',        dest='hostsmask',   type='string',       default=None,      help='Hosts mask')
Parser.add_option(      '--offline',     dest='offline',     action='store_true', default=False,     help='Send job offline (paused)')
Parser.add_option(      '--ignore',      dest='ignore',      action='store_true', default=False,     help='Ignore render Nimby and Paused states')
Parser.add_option('-V', '--verbose',     dest='verbose',     action='store_true', default=False,     help='Verbose mode')
Parser.add_option('-D', '--debug',       dest='debug',       action='store_true', default=False,     help='Debug mode (verbose mode, no commands execution)')
(Options, Args) = Parser.parse_args()

if len(Args) < 1:
    Parser.error('You should specify command at least.')

if Options.debug:
    Options.verbose = True

Renders = []
if Options.hostsmask:
    for render in afcmd.getRenderList(Options.hostsmask):
        Renders.append(render.name)
    Renders.sort()
else:
    if len(Args) < 2:
        Parser.error('You should specify renders to run on.')
    Renders = Args[:-1]

if len(Renders) == 0:
    Parser.error("No renders found.")

Command = Args[-1]
JobName = Options.jobname
if JobName is None:
    JobName = Command.split(' ')[0]

if Options.verbose:
    print('JobName: %s' % JobName)
    print('Renders: %s' % (','.join(Renders)))
    print('Command: %s' % Command)

job = af.Job(JobName)
job.setMaintenance()

block = af.Block('Maintenance', Options.service)
block.setParser(Options.parser)
block.setCommand(Command)

for render in Renders:
    task = af.Task(render)
    block.tasks.append(task)

job.blocks.append(block)

if Options.maxruntasks:
    job.setMaxRunningTasks(Options.maxruntasks)

if Options.offline:
    job.offline()

if Options.ignore:
    job.setIgnoreNimby()
    job.setIgnorePaused()

if Options.verbose:
    job.output()

if not Options.debug:
    job.send()
