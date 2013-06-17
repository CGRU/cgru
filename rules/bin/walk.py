#!/usr/bin/env python

import json, os, sys, time

from optparse import OptionParser

Parser = OptionParser( usage="%prog [options]\ntype \"%prog -h\" for help", version="%prog 1.0")

Parser.add_option('-p', '--path',    dest='path',       type  ='string',     default='.',                help='Path to walk.')
Parser.add_option('-o', '--output',  dest='output',     type  ='string',     default='.rules/walk.json', help='File to save results.')
Parser.add_option('-V', '--verbose', dest='verbose',    action='store_true', default=False,              help='Verbose mode.')
Parser.add_option('-D', '--debug',   dest='debug',      action='store_true', default=False,              help='Debug mode.')

(Options, Args) = Parser.parse_args()

if Options.path == '':
	print('ERROR: path is not specified.')
	sys.exit(1)

if Options.verbose:
	print('Path = "%s"' % Options.path)

if not os.path.isdir( Options.path):
	print('ERROR: path does not exist:')
	print( Options.path)
	sys.exit(1)

def listdir( i_path):
#	print( i_path)
#	sys.stdout.write('\r')
#	sys.stdout.write( i_path)

	out = dict()
	out['num_files'] = 0
	out['num_folders'] = 0
	out['size'] = 0

	cur = dict()
	cur['folders'] = dict()
	cur['files'] = dict()

	entries = os.listdir( i_path)

	for entry in entries:
		if entry == os.path.dirname( Options.output):
			continue

		path = os.path.join( i_path, entry)

		if os.path.isdir( path):
			out['num_folders'] += 1
			fout = listdir( path)
			cur['folders'][entry] = fout
			out['num_folders'] += fout['num_folders']
			out['num_files'] += fout['num_files']
			out['size'] += fout['size']

		if os.path.isfile( path):
			out['num_files'] += 1
			out['size'] += os.path.getsize( path)

	cur.update( out)

#	if i_path == Options.path: print( cur)

	filename = os.path.join( i_path, Options.output)
	if not os.path.isdir( os.path.dirname( filename)):
		os.makedirs( os.path.dirname( filename))

#	print( i_path, filename)
#	print( json.dumps( cur))
	file = open( filename, 'w')
	file.write( json.dumps( cur, indent=1))
	file.close()

	return out

time_start = time.time()
print('Started at: %s' % time.ctime( time_start))

print( listdir( Options.path))
#listdir( Options.path)

time_finish = time.time()

print('Finished at: %s' % time.ctime( time_finish))
print('Run time: %f seconds.' % (time_finish - time_start))

