import os
import optparse
import sys

import nuke

# Function to print message and exit with error:
def errorExit( msg):
   print 'Error: %s' % msg
   exit(1)

# Correct arguments for Nuke UNIX slaches:
def pathToUNIX( path):
   path = path.replace('\\','/')
   path = path[:1] + path[1:].replace('//','/')
   return path

# Parse arguments:
parser = optparse.OptionParser( usage="usage: %prog [options] (like nuke --help)", version="%prog 1.0")
parser.add_option('-x', '--xscene',       dest='xscene',       type='string', default='encode.nk', help='Path to scene to execute.')
parser.add_option('-f', '--fps',          dest='fps',          type='float',  default=24,          help='Frame rate.')
parser.add_option('-s', '--colorspace',   dest='colorspace',   type='string', default='',          help='Set color space.')
parser.add_option('-r', '--rnode',        dest='rnode',        type='string', default='Read1',     help='Node(s) to read a sequence.')
parser.add_option('-X', '--xnode',        dest='xnode',        type='string', default='Write1',    help='The name of node to execute.')
(options, args) = parser.parse_args()

# Check for an input sequence:
if len(args) < 1: errorExit('Sequence to encode not specified.')
sequence = args[0]

# Check for an output movie name:
if len(args) < 2: errorExit('Output movie name not specified.')
output = args[1]

# Get images pattern:
inputdir = os.path.dirname( sequence)
if not os.path.isdir( inputdir): errorExit('Images folder "%s" not founded.' % inputdir)
imagesname = os.path.basename( sequence)
pos = imagesname.find('.%')
if pos == -1: errorExit('Invalid input sequence.')
imagesext = imagesname[pos+2:]
imagesname = imagesname[0:pos]
pos = imagesext.find('d.')
if pos == -1: errorExit('Error: Invalid input sequence.')
imagesext = imagesext[pos+2:]

# Get images:
frame_first = -1
frame_last = -1
allfiles = os.listdir( inputdir)
allfiles.sort()
for afile in allfiles:
   if os.path.isdir(afile): continue
   if afile.find(imagesname) != 0: continue
   if afile.find(imagesext) == -1: continue
   if afile[0:len(imagesname)] != imagesname: continue
   if afile[-len(imagesext):] != imagesext: continue
   digits = afile[len(imagesname)+1:-len(imagesext)-1]
   number = -1
   try:
      number = int(digits)
   except:
      continue
   if frame_first == -1: frame_first = number
   frame_last = number
if frame_first == -1 or frame_last == -1: errorExit('Error: Invalid input sequence.')

# Correct arguments for Nuke UNIX slaches:
sequence = pathToUNIX( sequence)
output = pathToUNIX( output)
print 'Input:'
print sequence
print os.path.join(inputdir, imagesname) + '.[' + str(frame_first) + ' - ' + str(frame_last) + '].' + imagesext
print 'Output:'
print output

# Try to open scene:
if not os.path.isfile( options.xscene): errorExit('File "%s" not founded.' % options.xscene)
try: nuke.scriptOpen( options.xscene)
except: errorExit('Scene open error:\n' + str(sys.exc_info()[1]))

# Try to process read nodes:
readnodes = options.rnode.split(',')
for nodename in readnodes:
   readnode = nuke.toNode(nodename)
   if readnode is None: errorExit('Read "%s" not founded.' % nodename)
   if readnode.Class() != 'Read': errorExit('Node "%s" class is not "Read".' % nodename)
   readnode['file'].setValue( sequence)
   readnode['first'].setValue( frame_first)
   readnode['last'].setValue( frame_last)
   if options.colorspace != '': readnode['colorspace'].setValue( options.colorspace)

# Try to process write nodes:
writenode = nuke.toNode( options.xnode)
if writenode is None: errorExit('Node "%s" not founded.' % options.xnode)
if writenode.Class() != 'Write': errorExit('Node "%s" class is not "Write".' % options.xnode)
writenode['file'].setValue( output)
writenode['fps'].setValue( options.fps)

# Execute process:
nuke.execute( writenode, frame_first, frame_last)

