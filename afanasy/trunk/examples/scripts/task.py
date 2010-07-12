#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import random
import signal
import sys
import time

import parsers.parser

def interrupt( signum, frame):
   exit('\nInterrupt signal received...')

# Set interrupt function:
signal.signal( signal.SIGTERM, interrupt)
signal.signal( signal.SIGABRT, interrupt)
signal.signal( signal.SIGINT,  interrupt)

from time import strftime
print 'Started at ' + strftime('%A %d %B %H:%M:%S')

from optparse import OptionParser
parser = OptionParser(usage='usage: %prog [options]', version='%prog 1.0')
parser.add_option('-s', '--start',     dest='start',     type='int',    default=1,  help='start frame number')
parser.add_option('-e', '--end',       dest='end',       type='int',    default=2,  help='end frame number')
parser.add_option('-t', '--time',      dest='timesec',   type='float',  default=2,  help='time per frame in seconds')
parser.add_option('-r', '--randtime',  dest='randtime',  type='float',  default=0,  help='random time per frame in seconds')
parser.add_option('-f', '--file',      dest='filename',  type='string', default='', help='file name to open and print in stdout')
parser.add_option('-v', '--verbose',   dest='verbose',   type='int',    default=0,  help='verbose')
parser.add_option('-c', '--cpunum',    dest='cpunum',    type='int',    default=0,  help='number of processors to use')
parser.add_option('-H', '--HOSTS',     dest='hosts',     type='string', default='', help='hosts list')
(options, args) = parser.parse_args()
frame_start = options.start
frame_end   = options.end
timesec     = options.timesec
randtime    = options.randtime
filename    = options.filename
verbose     = options.verbose
cpunum      = options.cpunum

print 'COMMAND:'
print sys.argv
print 'WORKING DIRECTORY:'
print os.getcwd()

if( filename != ''):
   print 'FILE:'
   print filename
   f = open( filename)
   try:
      for line in f:
         print line
   finally:
      f.close()

sleepsec = (timesec + randtime*random.random()) / 100 / (verbose+1)

for f in range( frame_start, frame_end + 1):
   print 'FRAME: ' + str(f)
   time.sleep(sleepsec)
   for p in range(100):
      print 'PROGRESS: ' + str(p+1)+'%'
      for v in range( verbose):
         print str(f) + ': ' + str(p) + ': ' + str(v) + ': QWERTYUIOPASDFGHJKLZXCVBNM1234567890qwertyuiopasdfghjklzxcvbnm'
#         sys.stdout.flush()
         time.sleep(sleepsec)
      time.sleep(sleepsec)
      sys.stdout.flush()

#print parsers.parser.str_warning
print parsers.parser.str_error
#print parsers.parser.str_badresult
sys.stdout.flush()
print 'Finished at ' + strftime('%A %d %B %H:%M:%S')
