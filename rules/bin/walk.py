#!/usr/bin/env python

import json, os, sys, time

from optparse import OptionParser

Parser = OptionParser( usage="%prog [options]\ntype \"%prog -h\" for help", version="%prog 1.0")

Parser.add_option('-p', '--path',    dest='path',       type  ='string',     default='.',                help='Path to walk.')
Parser.add_option('-o', '--output',  dest='output',     type  ='string',     default='.rules/walk.json', help='File to save results.')
Parser.add_option('-m', '--mtime',   dest='mtime',      type  =int,          default=-1,                 help='Modification time difference.')
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

Progress = 0
TotalFiles = 0
CurFiles = 0

def listdir( i_path, i_curdepth = -1):
	global Progress
	global TotalFiles
	global CurFiles

	curdepth = i_curdepth + 1
	if Options.verbose:
		if curdepth < 4:
			print( i_path)

	if Options.mtime >= 0 and i_path != Options.path:
		filename = os.path.join( i_path, Options.output)
		if os.path.isfile( filename):
			if os.path.getmtime( i_path) - os.path.getmtime( filename ) < Options.mtime:
				if Options.verbose:
					print('Not modified %s' % i_path)
				file = open( filename, 'r')
				out = json.load( file)
				if 'files' in out: del out['files']
				if 'folders' in out: del out['folders']
				file.close()
				return out

	out = dict()
	out['num_files'] = 0
	out['num_folders'] = 0
	out['size'] = 0

	cur = dict()
	cur['folders'] = dict()
	cur['files'] = dict()

	try:
		entries = os.listdir( i_path)
	except:
		print( str(sys.exc_info()[1]))
		return None

	for entry in entries:
		if entry == os.path.dirname( Options.output):
			continue

		path = os.path.join( i_path, entry)

		if os.path.isdir( path):
			out['num_folders'] += 1
			fout = listdir( path, curdepth)
			if fout is not None:
				cur['folders'][entry] = fout
				out['num_folders'] += fout['num_folders']
				out['num_files'] += fout['num_files']
				out['size'] += fout['size']

		if os.path.isfile( path):
			out['num_files'] += 1
			CurFiles += 1
			out['size'] += os.path.getsize( path)

	cur.update( out)
	if TotalFiles:
		cur_progress = int( 100.0 * CurFiles / TotalFiles )
		if cur_progress != Progress:
			Progress = cur_progress
			print('PROGRESS: %d%%' % Progress)

	filename = os.path.join( i_path, Options.output)
	if not os.path.isdir( os.path.dirname( filename)):
		try:
			os.makedirs( os.path.dirname( filename))
		except:
			print( str(sys.exc_info()[1]))
			return out

	try:
		file = open( filename, 'w')
		json.dump( cur, file, indent=1)
		file.close()
	except:
		print( str(sys.exc_info()[1]))

	return out

time_start = time.time()
print('Started at: %s' % time.ctime( time_start))

# Get old files count if any:
filename = os.path.join( Options.path, Options.output)
if os.path.isfile( filename):
	file = open( filename, 'r')
	total = json.load( file)
	if 'num_files' in total: TotalFiles = total['num_files']
	file.close()

if TotalFiles:
	print('Previous run files count: %d' % TotalFiles)

print( listdir( Options.path))

time_finish = time.time()

print('Finished at: %s' % time.ctime( time_finish))
print('Run time: %f seconds.' % (time_finish - time_start))

