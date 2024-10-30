#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import random
# import signal
import sys
import time


time_start = time.time()
print('Started at: %s' % time.ctime(time_start))
print('COMMAND:')
print(sys.argv)
print('WORKING DIRECTORY:')
print(os.getcwd())
print('ENVIRONMENT ("CG_"):')
for env in os.environ:
    if env.find('CG_') == 0:
       print('%s=%s' % (env, os.environ[env]))


def interrupt(signum, frame):
    exit('\nInterrupt signal received...')

# Set interrupt function:
# signal.signal( signal.SIGTERM, interrupt)
# signal.signal( signal.SIGABRT, interrupt)
#signal.signal( signal.SIGINT,  interrupt)

from optparse import OptionParser

Parser = OptionParser(usage='usage: %prog [Options]', version='%prog 1.0')
Parser.add_option('-s', '--start',     dest='start',     type='int',    default=1,    help='Start frame number')
Parser.add_option('-e', '--end',       dest='end',       type='int',    default=1,    help='End frame number')
Parser.add_option('-i', '--increment', dest='increment', type='int',    default=1,    help='Frame increment')
Parser.add_option('-t', '--time',      dest='timesec',   type='float',  default=2,    help='Time per frame in seconds')
Parser.add_option('-r', '--randtime',  dest='randtime',  type='float',  default=0,    help='Random time per frame in seconds')
Parser.add_option('-f', '--filesout',  dest='filesout',  type='string', default=None, help='File(s) to write (";" separated)')
Parser.add_option(      '--stdoutfile',dest='stdoutfile',type='string', default=None, help='File to read stdout from')
Parser.add_option(      '--imgres',    dest='imgres',    type='string', default='1920x1080',help='Image resolution to write')
Parser.add_option('-v', '--verbose',   dest='verbose',   type='int',    default=0,    help='Verbose')
Parser.add_option('-p', '--pkp',       dest='pkp',       type='int',    default=1,    help='Parser key percentage')
Parser.add_option('-H', '--hosts',     dest='hosts',     type='string', default=None, help='Hosts list for mutihost tasks')
Parser.add_option('--exitstatus',      dest='exitstatus',type='int',    default=0,    help='Good exit status')

Options, Args = Parser.parse_args()

frame_start = Options.start
frame_end = Options.end
frame_inc = Options.increment

ParserKeys = ['[ PARSER WARNING ]', '[ PARSER ERROR ]', '[ PARSER FATAL ERROR ]',
              '[ PARSER BAD RESULT ]', '[ PARSER FINISHED SUCCESS ]']

StdOut = None
if Options.stdoutfile:
    if not os.path.isfile(Options.stdoutfile):
        print('ERROR: File "%s" does not exist.')
        sys.exit(1)
    print('Reading stdout from "%s"' % Options.stdoutfile)
    with open(Options.stdoutfile) as file:
        StdOut = file.readlines()

# Check frame range settings:
if frame_end < frame_start:
    print('Error: frame_end(%d) < frame_start(%d)' % (frame_end, frame_start))
    frame_end = frame_start
    print('[ PARSER WARNING ]')

if frame_inc < 1:
    print('Error: frame_inc(%d) < 1' % frame_inc)
    frame_inc = 1
    print('[ PARSER WARNING ]')

frame = frame_start
parserKey_CurIndex = int(random.random() * 100) % len(ParserKeys)

while frame <= frame_end:
    print('FRAME: %s' % frame)

    if StdOut is None:
        for p in range(100):
            print('PROGRESS: {progress}%'.format(progress=p + 1))

            if p == 10:
                print('ACTIVITY: Generating')

            if p == 50:
                print('ACTIVITY: Rendering')
                print('REPORT: ' + str(random.random()))

            if p == 90:
                print('ACTIVITY: Finalizing')

            if random.random() * 100 * 100 < Options.pkp:
                print(ParserKeys[parserKey_CurIndex])
                parserKey_CurIndex += 1
                if parserKey_CurIndex >= len(ParserKeys):
                    parserKey_CurIndex = 0

            for v in range(Options.verbose):
                print(
                    '%s: %s: %s: QWERTYUIOPASDFGHJKLZXCVBNM1234567890qwertyuiopasd'
                    'fghjklzxcvbnm' % (frame, p, v)
                )

            sys.stdout.flush()
            time.sleep(.01 * (Options.timesec + Options.randtime * random.random()))

    else:
        for line in StdOut:
            print(line, end='')
            sys.stdout.flush()
            time.sleep(Options.timesec / len(StdOut))

    if Options.filesout:
        files = Options.filesout.split(';')
        for afile in files:

            dirout = os.path.dirname( afile)
            if len(dirout) and not os.path.isdir( dirout):
                os.makedirs( dirout)
            while afile.find('%') != -1:
                afile = afile % frame

            cmd = 'convert -verbose -size "%s" xc:Black -gravity Center -fill White -pointsize 300 -annotate +0+0 "%s" "%s"'
            cmd = cmd % (Options.imgres, ("Frame: %04d" % frame), afile)
            os.system(cmd)

            print('@IMAGE@' + afile)

    frame += frame_inc

    if StdOut is not None:
        break

time_finish = time.time()
print('Finished at: %s' % time.ctime(time_finish))
print('Running time = %d seconds.' % (time_finish - time_start))

sys.stdout.flush()

if Options.exitstatus != 0:
    print('Good exit status is "%d"' % Options.exitstatus)

sys.exit( Options.exitstatus)

