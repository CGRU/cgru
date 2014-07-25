# -*- coding: utf-8 -*-
import os
import optparse
import shutil
# import signal
import sys
import tempfile
import re

import nuke

from parsers import parser

tmpdir = None


def interrupt(signum, frame):
	"""Interrupt function to delete temp directory:
	"""
	print('\nInterrupt received...')
	if tmpdir is not None and os.path.isdir(tmpdir):
		shutil.rmtree(tmpdir)
	exit(1)


# Set interrupt function:
# signal.signal( signal.SIGINT,  interrupt)
# signal.signal( signal.SIGTERM, interrupt)
#signal.signal( signal.SIGABRT, interrupt)
#if sys.platform.find('win') != 0:
#   signal.signal( signal.SIGQUIT, interrupt)
#   signal.signal( signal.SIGHUP,  interrupt)

def errorExit(msg, deletetemp):
	"""Error function to print(message) and delete temp directory:
	"""
	print(msg)
	if deletetemp:
		print('Removing temp directory:')
		print(tmpdir)
		shutil.rmtree(tmpdir)
	exit(1)

# Parse arguments:
ArgsParser = optparse.OptionParser( usage="usage: %prog [options] (like nuke --help)", version="%prog 1.0")
ArgsParser.add_option('-x', '--xscene', dest='xscene', type='string', default='', help='Path to scene to execute')
ArgsParser.add_option('-X', '--xnode',  dest='xnode',  type='string', default='', help='The name of node to execute')
ArgsParser.add_option('-F', '--frange', dest='frange', type='string', default='', help='Frame range to render (Nuke syntax)')
ArgsParser.add_option(      '--notmpimage', dest='notmpimage', action='store_true', default=False, help='Do not use temporary image.')
ArgsParser.add_option(      '--nopathsmap', dest='nopathsmap', action='store_true', default=False, help='Do not use paths map.')
options, args = ArgsParser.parse_args()
xscene = options.xscene
xnode = options.xnode
srange = options.frange

# Check arguments:
if xscene == '':
	errorExit('Scene to execute is not specified.', False)

if xnode == '':
	errorExit('Node to execute is not specified.', False)

if srange == '':
	errorExit('Frame range is not specified.', False)

# Check frame range:
frange = re.findall(r'[0-9]+', srange)

if frange is None:
	errorExit('No numbers in frame range found', False)

if len(frange) == 2:
	frange.append('1')

if len(frange) != 3:
	errorExit(
		'Invalid frame range specified, type A-BxC - [from]-[to]x[by]',
		False
	)

try:
	ffirst = int(frange[0])
	flast = int(frange[1])
	fby = int(frange[2])
except Exception as e:
	errorExit(
		'%s\nInvalid frame range syntax, type A-BxC - [from]-[to]x[by]' % e,
		False
	)

# Check for negative numbers:
pos = srange.find(frange[0])
if pos > 0:
	if srange[pos - 1] == '-':
		ffirst = -ffirst
srange = srange[pos + len(frange[0]):]
pos = srange.find(frange[1])
if pos > 1:
	if srange[pos - 2:pos] == '--':
		flast = -flast

# Check first and last frame values:
if flast < ffirst:
	errorExit(
		'First frame (%(ffirst)d) must be grater or equal last frame '
		'(%(flast)d)' % vars(),
		False
	)

if fby < 1:
	errorExit(
		'By frame (%(fby)d) must be grater or equal 1' % vars(),
		False
	)

# Check scene file for existence:
if not os.path.isfile(xscene):
	errorExit('File "%s" not found.' % xscene, False)

# Create and check temp directory:
tmpdir = os.path.join(
	tempfile.gettempdir(),
	'.afrender.nuke.%s.%s' % (os.path.basename(xscene), os.getpid())
)

if os.path.exists(tmpdir):
	shutil.rmtree(tmpdir)

os.makedirs(tmpdir)
if os.path.isdir(tmpdir):
	print('Temp directory = "%s"' % tmpdir)
else:
	errorExit('Error creating temp directory.', False)

# Transfer scene paths:
if not options.nopathsmap:
	cgrupathmap = __import__('cgrupathmap', globals(), locals(), [])
	pm = cgrupathmap.PathMap(UnixSeparators=True, Verbose=True)
	if pm.initialized:
		pmscene = os.path.basename(xscene)
		pmscene = os.path.join(tmpdir, pmscene)
		pm.toClientFile(
			xscene,
			pmscene,
			SearchStrings=['file ', 'font ', 'project_directory '],
			Verbose=False
		)
		xscene = pmscene
		print('Scene paths mapped: "%s"' % xscene)

# Try to open scene:
print('Opening ' + xscene)
try:
	nuke.scriptOpen(xscene)
except Exception as e:
	errorExit(
		'Scene open error:\n%s' % e,
		True
	)

# Try to process write node:
writenode = nuke.toNode(xnode)
if writenode is None:
	errorExit('Node "%s" not found.' % xnode, True)

if writenode.Class() != 'Write':
	errorExit('Node "%s" class is not "Write".' % xnode, True)

# Get file knob which can be a proxy:
if nuke.toNode('root').knob('proxy').value():
	fileknob = writenode['proxy']
else:
	fileknob = writenode['file']

# Get views and images folders:
imagesdirs = []
filenames = []
views_str = None
views = []
views_num = 0
try:
	views_str = writenode['views'].value()
	print('Views = "%s"' % views_str)
	for view in views_str.split(' '):
		view = view.strip()
		if view != '':
			if not view in nuke.views():
				print('Warning: Skipping invalid view: "%s"' % view)
				print(parser.str_warning)
				continue
			views_num += 1
			views.append(view)
			octx = nuke.OutputContext()
			octx.setView(1 + nuke.views().index(view))
			filename = fileknob.getEvaluatedValue(octx)
			imagesdirs.append(os.path.dirname(filename))
			filenames.append(filename)
except Exception as e:
	errorExit('Can`t process views on "%s" write node:\n%s' % (xnode, e), True)

# Check for valid view found:
if views_num < 1:
	errorExit('Can`t find valid views on "%s" write node.' % xnode, True)

# Check for multiview file (stereo EXR)
multiview_file = False
if views_num > 1:
	name = None
	multiview_file = True
	for img in filenames:
		if name is None:
			name = img
			continue
		if name != img:
			multiview_file = False
			break

# Change render images folder to temporary:
if not options.notmpimage:
	try:
		filepath = fileknob.value()
		# Nuke paths has only UNIX slashes, even on MS Windows platform
		if sys.platform.find('win') == 0:
			filepath = filepath.replace('/', '\\')
		imagesname = os.path.basename(filepath)
		tmppath = os.path.join(tmpdir, imagesname)
		# Nuke paths has only UNIX slashes, even on MS Windows platform
		if sys.platform.find('win') == 0:
			tmppath = tmppath.replace('\\', '/')
		fileknob.setValue(tmppath)
	except Exception as e:
		errorExit(
			'File operations error on "%s" write node:\n%s' % (xnode, e),
			True
		)

print('Number of views = %d' % views_num)

# Render frames cycle:
exitcode = 0
frame = ffirst
while frame <= flast:
	print('Rendering frame %d:' % frame)
	sys.stdout.flush()

	# Iterate views:
	view_num = 0
	for view in views:
		if views_num > 1:
			# 'EXECUTING VIEW: "%s"' - needed for Afanasy to parse
			#                          task activity
			if multiview_file is False:
				print('EXECUTING VIEW "%s":' % view)
			else:
				print('Trying to execute several views in the same file.')
			sys.stdout.flush()

		# Try to execute write node:
		try:
			if nuke.env['NukeVersionMajor'] < 6:
				nuke.execute(writenode.name(), frame, frame)
			else:
				if multiview_file:
					nuke.execute(writenode, frame, frame, 1, views)
				else:
					nuke.execute(writenode, frame, frame, 1, [view])
		except Exception as e:
			print('Node execution error:')
			print(str(e))
			exitcode = 1

		if options.notmpimage:
			if multiview_file:
				break
			else:
				continue

		# Copy image files from temp directory:
		allitems = os.listdir(tmpdir)
		moveditems = 0
		for item in allitems:
			if item.rfind('.tmp') == len(item) - 4:
				continue

			if item.rfind('.nk') == len(item) - 3:
				continue

			src = os.path.join(tmpdir, item)
			dest = os.path.normpath(
				imagesdirs[view_num] + '/' + item
			)

			# Delete old image if any:
			if os.path.isfile(dest):
				try:
					print('Deleting old "%s"' % dest)
					os.remove(dest)
				except Exception as e:
					print(str(e))
					print('Unable to remove destination file:')
					print(dest)
					exitcode = 1

			# Move temporary image:
			try:
				print('Moving "%s"' % dest)
				shutil.move(src, imagesdirs[view_num])
			except Exception as e:
				print('File moved with error:')
				print(str(e))
				print(src)
				print(dest)

			# Check destination image:
			if not os.path.isfile(dest):
				print('Error: Destination file does not exist.')
				exitcode = 1
			else:
				moveditems += 1
				print('@IMAGE@' + dest)

		if moveditems < 1:
			print('Error: No images generated.')
			exitcode = 1
		else:
			print('Images generated: %d' % moveditems)

		sys.stdout.flush()

		if exitcode != 0:
			break

		view_num += 1

		if multiview_file:
			break

	if exitcode != 0:
		break

	frame += fby

# Remove temp directory:
shutil.rmtree(tmpdir)

# Exit:
exit(exitcode)

