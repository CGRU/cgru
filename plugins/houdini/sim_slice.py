#!/usr/bin/env hython

import os
from optparse import OptionParser
import parsers.hbatch

parser = OptionParser(usage="usage: %prog [options] hip_name rop_name", version="%prog 1.0")
parser.add_option("-s", "--start",     dest="start",     type="int",    help="start frame number")
parser.add_option("-e", "--end",       dest="end",       type="int",    help="end frame number")
parser.add_option("-t", "--take",      dest="take",      type="string", help="take name")
parser.add_option("-c", "--slice",     dest="slice",     type="int",    help="slice number")

(options, args) = parser.parse_args()

if len(args) < 2:
   parser.error( "At least one of mandatory rop_name or hip_name argument is missed.")
elif len(args) > 2:
   parser.error( "Too many arguments provided.")
else:
   hip = args[0]
   rop = args[1]

start    = options.start
end      = options.end
take     = options.take
slice    = options.slice

hou.hipFile.load(hip,True)

# Establish ROP to be used
if rop[0] != "/":
   rop = "/out/" + rop
ropnode = hou.node( rop)
if ropnode == None:
   raise hou.InvalidNodeName( rop + " rop node wasn't found")

# Trying to set ROP to output progress
drivertypename = ropnode.type().name()

if take != None and len(take) > 0:
   hou.hscript("takeset " + take)

# If end wasn't specified, render single frame
if end == None:
   end = start

os.putenv("AF_SLICE", str(slice))
os.putenv("AF_TRPORT", str(18000))
os.putenv("AF_TRADDRESS", 'localhost')

ropnode.render((start,end))
#while frame <= end:
#   print parsers.hbatch.keyframe + str(frame)
#   sys.stdout.flush()
#   frame += by

