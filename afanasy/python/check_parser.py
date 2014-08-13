#!/usr/bin/env python

import os
import subprocess
import sys

sys.path.append(os.path.abspath(sys.argv[0]))


def UsageExit(message=''):
	"""Missing DocString

	:param message:
	:return:
	"""
	if message != '':
		print('Error: %s' % message)

	print('Usage: %s [parser type] [frames number] [command line]' %
		  os.path.basename(sys.argv[0]))
	exit(1)


if len(sys.argv) < 4:
	UsageExit()

try:
	framesNum = int(sys.argv[2])
except:  # TODO: Too broad exception clause
	UsageExit('%s\nInvalid frames number.' % str(sys.exc_info()[1]))

parserType = sys.argv[1]
parserModule = __import__('parsers', globals(), locals(), [parserType])
cmd = 'parserModule.%s.%s()' % (parserType, parserType)
print(cmd)
parser = eval(cmd)

taskInfo = dict()
taskInfo['frames_num'] = framesNum
taskInfo['wdir'] = os.getcwd()
parser.setTaskInfo(taskInfo)

arguments = []
for i in range(3, len(sys.argv)):
	arguments.append(sys.argv[i])

process = subprocess.Popen(arguments, shell=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
#process = subprocess.Popen(' '.join(arguments), shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
process.stdin.close()

stdout = process.stdout
stderr = process.stderr
#stdout = process.stderr
#stderr = process.stdout

while True:
	stdout.flush()
	data = stdout.readline()
	if data is None: break
	if len(data) < 1: break

	sys.stdout.write(data)
	parser.parse( data,'mode')
	sys.stdout.write('Parse: %d%%: %d frame %d%%\n' %
		  (parser.percent, parser.frame, parser.percentframe))
	sys.stdout.flush()

print( stderr.read().replace('\r',''))

