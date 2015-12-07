#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import sys
import time

import cgruutils

from optparse import OptionParser

Parser = OptionParser(
	usage="%prog [options]\ntype \"%prog -h\" for help",
	version="%prog 1.0"
)

Parser.add_option('-o', '--output',   dest='output',   type = 'string',     default='.rules/walk.json', help='File to save results.')
Parser.add_option('-n', '--noupdate', dest='noupdate', action='store_true', default=False,              help='Skip update upfolders.')
Parser.add_option('-t', '--thumb',    dest='thumb',    type = 'int',        default=None,               help='Make thumbnail frequency.')
Parser.add_option('-V', '--verbose',  dest='verbose',  type = 'int',        default=0,                  help='Verbose mode.')
Parser.add_option('-D', '--debug',    dest='debug',    action='store_true', default=False,              help='Debug mode.')

Options, Args = Parser.parse_args()

Progress = 0
PrevFiles = None
CurFiles = 0
StartPath = '.'
ThumbFolderCount = 0
os.umask(0000)

print('{"walk":{')

def outInfo( i_key, i_msg):
	print(' "%s"\t:"%s",' % (i_key, i_msg))
	sys.stdout.flush()

def outStatus( i_status):
	print(' "status"\t:"%s"' % i_status)
	print('}}')

if len(Args):
	StartPath = Args[0]

outInfo('path',StartPath)

if not os.path.isdir(StartPath):
	outInfo('error','Start path does not exists.')
	outStatus('error')
	sys.exit(1)

def jsonLoad(i_filename):
	if not os.path.isfile(i_filename):
		return None

	try:
		file = open(i_filename, 'r')
	except:
		outInfo('error_file_open',str(sys.exc_info()[1]))
		return None

	obj = None
	try:
		obj = json.load(file)
	except:
		outInfo('error_file_json',str(sys.exc_info()[1]))
		obj = None

	file.close()

	return obj

def checkDict(io_dict):
	if not 'folders' in io_dict:
		io_dict['folders'] = dict()
	if not 'files' in io_dict:
		io_dict['files'] = dict()
	num_keys = ['num_files', 'num_folders','num_images', 'size', 'num_files_total',
				'num_folders_total', 'size_total']
	for key in num_keys:
		if not key in io_dict:
			io_dict[key] = 0


def walkdir(i_path, i_subwalk, i_curdepth=0):
	global Progress
	global PrevFiles
	global CurFiles
	global ThumbFolderCount

	if Options.verbose > i_curdepth and i_subwalk:
		outInfo('cur_path',i_path)

	out = dict()
	checkDict(out)

	try:
		entries = os.listdir(i_path)
	except:
		outInfo('error_listdir',str(sys.exc_info()[1]))
		return None

	for entry in entries:
		# Skip result folder (.rules):
		if entry == os.path.dirname(Options.output):
			continue

		path = os.path.join(i_path, entry)

		# We are not walking in links:
		if os.path.islink(path):
			continue

		if os.path.isdir(path):
			out['num_folders'] += 1
			out['num_folders_total'] += 1

			fout = None
			if i_subwalk:
				# Recursively walk in a subfolder:
				fout = walkdir(path, True, i_curdepth + 1)
			else:
				# Load previous walk data:
				fout = jsonLoad(os.path.join(path, Options.output))

			if fout is not None:
				checkDict(fout)

				# We do not need info for each subfolder in a child folder:
				del fout['files']
				del fout['folders']
				out['folders'][entry] = fout

				out['num_folders_total'] += fout['num_folders_total']
				out['num_files_total'] += fout['num_files_total']
				out['size_total'] += fout['size_total']

		if os.path.isfile(path):
			CurFiles += 1
			if entry[0] != '.':
				out['num_files'] += 1
				if cgruutils.isImageExt( path):
					if Options.thumb is not None:
						if out['num_images'] == 0:
							if ThumbFolderCount % Options.thumb == 0:
								print('@IMAGE!@'+path)
								sys.stdout.flush()
							ThumbFolderCount += 1
					out['num_images'] += 1
			out['num_files_total'] += 1
			out['size_total'] += os.path.getsize(path)
			out['size'] += os.path.getsize(path)

	# Just output progress:
	if PrevFiles:
		cur_progress = int(100.0 * CurFiles / PrevFiles)
		if cur_progress != Progress:
			Progress = cur_progress
			outInfo('progress','PROGRESS: %d%%' % Progress)

	# Store current walk data:
	filename = os.path.join(i_path, Options.output)

	if not os.path.isdir(os.path.dirname(filename)):
		try:
			os.makedirs(os.path.dirname(filename))
		except:
			outInfo('error_make_dir',str(sys.exc_info()[1]))

	if os.path.isdir(os.path.dirname(filename)):
		try:
			with open(filename, 'w') as f:
				json.dump(out, f, indent=1)
		except:
			outInfo('error_file_write',str(sys.exc_info()[1]))

	return out


# #############################################################################
time_start = time.time()
outInfo('time_start', time.ctime(time_start))

# Get old files count if any:
prev = jsonLoad(os.path.join(StartPath, Options.output))
if prev is not None:
	if 'num_files_total' in prev:
		checkDict(prev)
		PrevFiles = prev['num_files_total']

if PrevFiles:
	outInfo('previous','%s files, %s folders, %s bytes' % (
		cgruutils.sepThousands(prev['num_files_total']), cgruutils.sepThousands(prev['num_folders_total']),
		cgruutils.sepThousands(prev['size_total'])))

# Walk in subfolders:
walk = walkdir(StartPath, True)

# Calculate difference with previous
d_files = None
d_folders = None
d_size = None
if PrevFiles:
	d_files = walk['num_files_total'] - prev['num_files_total']
	d_folders = walk['num_folders_total'] - prev['num_folders_total']
	d_size = walk['size_total'] - prev['size_total']


# Update parent folders:
if not Options.noupdate:
	curpath = StartPath
	PrevFiles = None
	while curpath != '/' and curpath != '':
		# Go one folder upper:
		uppath = os.path.dirname(curpath)
		if uppath == curpath or uppath == '' or uppath == '/':
			break
		curpath = uppath

		outInfo('updating', curpath)
		walkdir(curpath, False)


# Output statistics:
time_finish = time.time()
outInfo('time_finish', time.ctime(time_finish))
outInfo('processed','%s files, %s folders, %s bytes' % (
	cgruutils.sepThousands(walk['num_files_total']),
	cgruutils.sepThousands(walk['num_folders_total']),
	cgruutils.sepThousands(walk['size_total']))
)
print('"walk":%s,' % json.dumps(walk,indent=1))

if d_files is not None:
	outInfo('delta','%s files, %s folders, %s bytes' %
		(cgruutils.sepThousands(d_files), cgruutils.sepThousands(d_folders), cgruutils.sepThousands(d_size)))

sec = time_finish - time_start
hrs = int(sec / 3600)
sec -= hrs * 3600
mns = int(sec / 60)
sec -= mns * 60
msc = int(1000.0 * sec - int(sec))
sec = int(sec)
outInfo('time_run','%02d:%02d:%02d.%03d' % (hrs, mns, sec, msc))

outStatus('success')
