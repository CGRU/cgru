#!/usr/bin/env python

import json, hashlib, os, subprocess, sys, time
import traceback

from optparse import OptionParser
Parser = OptionParser(usage="%prog [options] output\nType \"%prog -h\" for help", version="%prog 1.0")
Parser.add_option('-i', '--input',   dest='input' ,  type  ='string',     default='',    help='Input file')
Parser.add_option('-o', '--output',  dest='output',  type  ='string',     default='',    help='Output file')
Parser.add_option('-t', '--type',    dest='type',    type  ='string',     default='md5', help='Input file')
Parser.add_option('-V', '--verbose', dest='verbose', action='store_true', default=False, help='Verbose mode')
Parser.add_option('-D', '--debug',   dest='debug',   action='store_true', default=False, help='Debug mode')
(Options, args) = Parser.parse_args()
if Options.debug: Options.verbose = True


def errorExit( i_msg = None):
	if i_msg:	
		print('Error: ' + i_msg)
	if sys.exc_info()[1]:
		traceback.print_exc(file=sys.stdout);
	sys.exit(1)

def makeDir( i_folder):
	if( len( i_folder) == 0 ): return
	if os.path.isdir( i_folder ): return
	print('Creating folder:\n' + i_folder )
	if Options.debug: return
	try:
		os.makedirs( i_folder)
	except:
		errorExit('Can`t create folder "%s"' % i_folder )


if Options.input == '': errorExit('Input file is not specified')
if not os.path.isfile( Options.input ): errorExit('Input file does not exist:\n' + Options.input)
file_in_size = -1
try:
	file_in_size = os.path.getsize( Options.input)
except:
	errorExit('Can`t access input file:\n' + Options.input)
if file_in_size == 0:
	errorExit('Input file is empty:\n' + Options.input)


if Options.output == '':
	Options.output = os.path.dirname( Options.input)
	Options.output = os.path.join( Options.output, '.rules')
	Options.output = os.path.join( Options.output, 'walk.json')
makeDir( os.path.dirname( Options.output))


print('Input:  %s - %d bytes' % ( Options.input, file_in_size))
print('Output: ' + Options.output)
print('Type:   ' + Options.type)
time_start = time.time()
print('Started at: %s' % time.ctime( time_start))


chsum = None
if Options.type == 'md5':
	chsum = hashlib.md5()
else:
	errorExit('Unsupported checksum type: "%s"' % Options.type)


file_in = open( Options.input, 'rb')
read_len = 0
progress = -1
while True:
	data = file_in.read( 1000000)
	if len( data) <= 0: break

	chsum.update( data)

	read_len += len(data)
	new_progress = int(100.0 * read_len / file_in_size)
	if new_progress != progress:
		progress = new_progress
		print('PROGRESS: %d%%' % progress)

file_in.close()
result = chsum.hexdigest()
print( result)

obj = dict()
if os.path.isfile( Options.output):
	file_out = open( Options.output, 'r')
	obj = json.load( file_out)
	file_out.close()
	#print( json.dumps( obj))

file_in_name = os.path.basename( Options.input)
if not 'files' in obj: obj['files'] = dict()
if not file_in_name in obj['files']: obj['files'][file_in_name] = dict()
if not 'checksum' in obj['files'][file_in_name]: obj['files'][file_in_name]['checksum'] = dict()
obj['files'][file_in_name]['checksum'][Options.type] = result
obj['files'][file_in_name]['checksum']['time'] = time.time()
result = json.dumps( obj, indent=1)

file_out = open( Options.output, 'w')
file_out.write( result)
file_out.close()

# Output running time:
time_finish = time.time()
print('Finished at: %s' % time.ctime( time_finish))
sec = time_finish - time_start
hrs = int( sec / 3600 )
sec -= hrs * 3600
mns = int( sec / 60 )
sec -= mns * 60
msc = int( 1000.0 * sec - int( sec))
sec = int( sec)
print('Run time: %02d:%02d:%02d.%03d' % (hrs, mns, sec, msc))

