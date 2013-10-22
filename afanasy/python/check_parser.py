#!/usr/bin/env python

import os
import subprocess
import sys

sys.path.append( os.path.abspath(sys.argv[0]))

def UsageExit( message = ''):
   if message != '': print('Error: ' + message)
   print('Usage: ' + os.path.basename(sys.argv[0]) + ' [parser type] [frames number] [command line]')
   exit(1)

if len(sys.argv) < 4: UsageExit()

try:
   framesNum = int(sys.argv[2])
except:
   UsageExit( str(sys.exc_info()[1]) + '\nInvalid frames number.')

parserType = sys.argv[1]
parserModule = __import__('parsers', globals(), locals(), [parserType])
cmd = 'parserModule.%s.%s()' % ( parserType, parserType)
print( cmd )
parser = eval( cmd)

taskInfo = dict()
taskInfo['frames_num'] = framesNum
taskInfo['wdir'] = os.getcwd()
parser.setTaskInfo( taskInfo)

arguments = []
for i in range( 3, len(sys.argv)): arguments.append( sys.argv[i])

output = subprocess.Popen( arguments, stdout=subprocess.PIPE).stdout

for line in output:
   print( line,)
   parser.parse('test', line)
   print('Parse: %d%%: %d frame %d%%' % (parser.percent, parser.frame, parser.percentframe))
   
output.close()
