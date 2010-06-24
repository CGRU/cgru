#!/usr/bin/env python
import mantra
import sys, os

# Process arguments:
divx=int(sys.argv[1])
divy=int(sys.argv[2])
tile=int(sys.argv[3])

# Calculate tile crop region:
numx = tile % divx;
numy = tile / divx;
stepx = 1.0 / divx;
stepy = 1.0 / divy;
tileCrop = [ numx*stepx,(numx+1)*stepx, numy*stepy,(numy+1)*stepy ]

# Store filtered names:
filteredNames = []

#def getTmpName( newName ):
#   pos = newName.rfind("/")
#   newNames[-1] = tmpDir + newName[pos:]
#   return newNames[-1]

# This one is called for each image plane - change name here
# NB! only additional image plane files can be changed here - main image filename will have to be set through command line
def filterPlane():
   oldName = mantra.property('plane:planefile')[0]
   newName = oldName
   if not oldName in filteredNames: newName += '.tile_%d.exr' % tile
   filteredNames.append( newName)
   print 'Tile filename:\n' + newName
#   if useTmpDir:
   if False:
      tmpName = getTmpName( newName )
      mantra.setproperty('plane:planefile', tmpName)
   else:
      mantra.setproperty('plane:planefile', newName)

# A callback to change image crop and write down main image filename:
def filterCamera():
   oldCrop = mantra.property('image:crop')
   newCrop = [ max(tileCrop[0],oldCrop[0]),min(tileCrop[1],oldCrop[1]),  max(tileCrop[2],oldCrop[2]),min(tileCrop[3],oldCrop[3])]
   mantra.setproperty('image:crop', newCrop)
