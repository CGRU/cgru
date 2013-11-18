#!/usr/bin/env python

import json, os, sys, time

from optparse import OptionParser

Parser = OptionParser( usage="%prog [options]\ntype \"%prog -h\" for help", version="%prog 1.0")

Parser.add_option('-o', '--output',  dest='output',     type = 'string',     default='.rules/walk.json', help='File to save results.')
Parser.add_option('-V', '--verbose', dest='verbose',    type = 'int',        default=0,                  help='Verbose mode.')
Parser.add_option('-D', '--debug',   dest='debug',      action='store_true', default=False,              help='Debug mode.')

(Options, Args) = Parser.parse_args()

Progress = 0
PrevFiles = None
CurFiles = 0
StartPath = '.'

if len( Args):
	StartPath = Args[0]

if not os.path.isdir( StartPath):
	print('ERROR: path does not exist:')
	print( StartPath)
	sys.exit(1)

def jsonLoad( i_filename):
	if not os.path.isfile( i_filename):
		return None

	try:
		file = open( i_filename, 'r')
	except:
		print( str(sys.exc_info()[1]))
		return None

	obj = None
	try:
		obj = json.load( file)
	except:
		print('ERROR: %s' % i_filename)
		print( str(sys.exc_info()[1]))
		obj = None

	file.close()

	return obj

def walkdir( i_path, i_maxdepth = -1, i_curdepth = -1):
	global Progress
	global PrevFiles
	global CurFiles

	curdepth = i_curdepth + 1
	if Options.verbose > curdepth:
		print( i_path)

	out = jsonLoad( os.path.join( i_path, Options.output))
	if i_maxdepth >= 0 and curdepth > i_maxdepth:
		if out is None: return None
		if 'files' in out: del out['files']
		if 'folders' in out: del out['folders']
		return out

	cur = dict()
	cur['folders'] = dict()
	cur['files'] = dict()

	if out is None:
		out = dict()
	else:
		if 'files' in out: cur['files'] = out['files']

	out['num_files'] = 0
	out['num_folders'] = 0
	out['size'] = 0

	try:
		entries = os.listdir( i_path)
	except:
		print( str(sys.exc_info()[1]))
		return None

	for entry in entries:
		if entry == os.path.dirname( Options.output): continue

		path = os.path.join( i_path, entry)

		if os.path.islink( path): continue

		if os.path.isdir( path):
			out['num_folders'] += 1
			fout = walkdir( path, i_maxdepth, curdepth)
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
	if PrevFiles:
		cur_progress = int( 100.0 * CurFiles / PrevFiles )
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

def sepTh( i_int):
	s = str( int(i_int))
	o = ''
	for i in range( 0, len( s)):
		o += s[len(s) - i - 1]
		if (i+1) % 3 == 0: o += ' '
	s = ''
	for i in range( 0, len( o)):
		s += o[len(o) - i - 1]
	return s

time_start = time.time()
print('Started at: %s' % time.ctime( time_start))

# Get old files count if any:
prev = jsonLoad( os.path.join( StartPath, Options.output))
if prev is not None:
	if 'num_files' in prev:
		PrevFiles = prev['num_files']

if PrevFiles:
	print('Previous run: %s files, %s folders, %s bytes' % ( sepTh( prev['num_files']), sepTh( prev['num_folders']), sepTh( prev['size'])))

# Walk in subfolders:
walk = walkdir( StartPath)

# Calculate difference with previous
d_files = None
d_folders = None
d_size = None
if PrevFiles is not None:
	d_files   = walk['num_files']   - prev['num_files']
	d_folders = walk['num_folders'] - prev['num_folders']
	d_size    = walk['size']        - prev['size']

# Walk in parent folders:
curpath = os.path.abspath( StartPath )
PrevFiles = None
while curpath != '/':
	uppath = os.path.dirname( curpath)
	if uppath == curpath: break
	curpath = uppath
	if not os.path.isfile( os.path.join( curpath, Options.output)): break

	print('Updating: %s' % curpath)
	walkdir( curpath, 0)


# Output statistics:
time_finish = time.time()
print('Finished at: %s' % time.ctime( time_finish))
print('Result: %s files, %s folders, %s bytes' % ( sepTh( walk['num_files']), sepTh( walk['num_folders']), sepTh( walk['size'])))
if d_files is not None:
	print('Delta: %s files, %s folders, %s bytes' % ( sepTh( d_files), sepTh( d_folders), sepTh( d_size)))

sec = time_finish - time_start
hrs = int( sec / 3600 )
sec -= hrs * 3600
mns = int( sec / 60 )
sec -= mns * 60
msc = int( 1000.0 * sec - int( sec))
sec = int( sec)
print('Run time: %02d:%02d:%02d.%03d' % (hrs, mns, sec, msc))

