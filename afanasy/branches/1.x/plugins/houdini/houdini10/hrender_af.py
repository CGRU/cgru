#!/usr/bin/env hython

import sys
from optparse import OptionParser
import parsers.hbatch

parser = OptionParser(usage="usage: %prog [options] hip_name rop_name", version="%prog 1.0")
parser.add_option("-s", "--start",     dest="start",     type="float",   help="start frame number")
parser.add_option("-e", "--end",       dest="end",       type="float",   help="end frame number")
parser.add_option("-b", "--by",        dest="by",        type="float",   help="frame increment")
parser.add_option("-t", "--take",      dest="take",      type="string", help="take name")
parser.add_option("-o", "--out",       dest="output",    type="string", help="output file")
parser.add_option(      "--numcpus",   dest="numcpus",   type="int",    help="number of CPUs")
parser.add_option("-i", "--ignore_inputs",    action="store_true", dest="ignore_inputs", default=False, help="Ignore inputs")

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
by       = options.by
take     = options.take
numcpus  = options.numcpus
output   = options.output
ignoreInputs = options.ignore_inputs
#print ignore_inputs

hou.hipFile.load(hip,True)

# Establish ROP to be used
if rop[0] != "/":
   rop = "/out/" + rop
ropnode = hou.node( rop)
if ropnode == None:
   raise hou.InvalidNodeName( rop + " rop node wasn't found")

# Trying to set ROP to block until render comletes
#block = ropnode.parm('soho_foreground')
#if block != None:
   #value = block.eval()
   #if value != None:
      #if value != 1:
         #try:
            #block.set( 1)
         #except:
            #print 'Failed to set node blocking.'

# Trying to set ROP to output progress
drivertypename = ropnode.type().name()
if drivertypename == 'ifd':
   progress = ropnode.parm('vm_alfprogress')
   if progress != None:
      value = progress.eval()
      if value != None:
         if value != 1:
            print 'Trying to set "Alfred Style Progress" on mantra'
            try:
               progress.set( 1)
            except:
               print 'Failed, frame progress not available.'
   if numcpus != None:
      paramcmd = ropnode.parm('soho_pipecmd')
      if paramcmd != None:
         command = paramcmd.eval()
         if command != None:
            print 'Trying to set mantra threads to ' + str(numcpus)
            command = command.replace('mantra', 'mantra -j ' + str(numcpus), 1)
            try:
               paramcmd.set( command)
               print '   Command changed to:'
               print command
            except:
               print 'Failed, number of mantra theads not changed.'

elif drivertypename == 'rib':
   progress = ropnode.parm('soho_pipecmd')
   if progress != None:
      command = progress.eval()
      if command != None:
         if command.find('-progress') == -1:
            print 'Trying to set "-progress" on prman'
            command = command.replace('prman', 'prman -progress', 1)
            try:
               progress.set( command)
            except:
               print 'Failed, frame progress not available.'


if take != None and len(take) > 0:
   hou.hscript("takeset " + take)

# If end wasn't specified, render single frame
if end == None:
   end = start

render_output = None
if output != None and len(output) > 0:
   render_output = output

frame = start
while frame <= end:
   render_range = ( frame, frame, by)
   print parsers.hbatch.keyframe + str(frame)
   sys.stdout.flush()
   ropnode.render( frame_range=render_range, output_file=render_output, method=hou.renderMethod.FrameByFrame, ignore_inputs=ignoreInputs)
   frame += by

