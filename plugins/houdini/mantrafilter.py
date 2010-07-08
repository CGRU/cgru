#!/usr/bin/env python
import mantra
import sys, os, shutil

tmpdir = ''
tilerender = False

# Process arguments:
argspos = 1

if len(sys.argv) > 3:
   divx=int(sys.argv[1])
   divy=int(sys.argv[2])
   tilenum=int(sys.argv[3])
   argspos = 4
   tilerender = True
   # Calculate tilenum crop region:
   numx = tilenum % divx;
   numy = tilenum / divx;
   stepx = 1.0 / divx;
   stepy = 1.0 / divy;
   tilecrop = [ numx*stepx,(numx+1)*stepx, numy*stepy,(numy+1)*stepy ]

if len( sys.argv) > argspos:
   tmpdir = sys.argv[argspos]

# Store filtered names:
filteredNames = []

# Store images dir:
imagesdir = ''

# This one is called for each image plane - change name here
# NB! only additional image plane files can be changed here - main image filename will have to be set through command line
def filterPlane():
   global imagesdir
   filename = mantra.property('plane:planefile')[0]
   if not filename in filteredNames:
      if imagesdir == '':
         imagesdir = os.path.dirname( filename)
      if tilerender:
         filename += '.tile_%d.exr' % tilenum
      if tmpdir != '':
         pos = filename.rfind("/")
         filename = tmpdir + filename[pos:]
   filteredNames.append( filename)
   print 'Mantra plane filename:\n' + filename
   mantra.setproperty('plane:planefile', filename)

# A callback to change image crop and write down main image filename:
def filterCamera():
   if tilerender:
      oldcrop = mantra.property('image:crop')
      newcrop = [ max(tilecrop[0],oldcrop[0]),min(tilecrop[1],oldcrop[1]),  max(tilecrop[2],oldcrop[2]),min(tilecrop[3],oldcrop[3])]
      mantra.setproperty('image:crop', newcrop)

def filterQuit():
   if tmpdir == '': return
   if imagesdir == '': return
   # Copy image files from temp directory:
   allitems = os.listdir( tmpdir)
   exitcode = 0
   for item in allitems:
      src  = os.path.join( tmpdir,    item)
      dest = os.path.join( imagesdir, item)
      if os.path.isfile( dest):
         try:
            print 'Deleting old "%s"' % dest
            os.remove( dest)
         except:
            print str(sys.exc_info()[1])
            print 'Unable to remove destination file:'
            print dest
            exitcode = 1
      try:      
         print 'Moving "%s"' % dest
         shutil.move( src, imagesdir)
      except:
         print 'File moved with error:'
         print str(sys.exc_info()[1])
         print src
         print dest
      if not os.path.isfile( dest):
         print 'Error: Destination file does not exist.'
         exitcode = 1
   if exitcode != 0: sys.exit(1)
